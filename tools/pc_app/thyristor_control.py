"""Программа управления контроллером тиристора по RS485 (Modbus RTU).

Запуск:  python thyristor_control.py
Зависимости: PyQt5, pyserial (см. requirements.txt).
"""

import sys

import serial
import serial.tools.list_ports
from PyQt5 import QtCore, QtWidgets

HOLDING_ANGLE = 0
HOLDING_PULSE_US = 1
HOLDING_WORK_MODE = 2
HOLDING_DURATION_MS = 3
HOLDING_MAX_RUN_S = 4
HOLDING_FREQ_DEV = 5
HOLDING_CONTROL_MODE = 6
HOLDING_BAUD_INDEX = 7
HOLDING_ADDRESS = 8
HOLDING_PARITY = 9
HOLDING_COMMAND = 10

HOLDING_COUNT = 11
INPUT_COUNT = 13

CMD_START = 1
CMD_STOP = 2
CMD_SAVE = 3

WORK_MODES = ["CONTINUOUS", "FIRST WAVE", "TIMED"]
CONTROL_MODES = ["LOCAL", "REMOTE"]
PARITIES = ["NONE", "EVEN", "ODD"]
BAUD_RATES = [9600, 19200, 38400, 57600, 115200]
DEVICE_STATES = ["READY", "WAIT SYNC", "WAIT ZERO", "TEST", "FINISHED"]


class ModbusError(Exception):
    pass


def crc16(data: bytes) -> int:
    crc = 0xFFFF

    for byte in data:
        crc ^= byte

        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1

    return crc


class ModbusClient:
    """Минимальный Modbus RTU master поверх pyserial."""

    def __init__(self, port: str, baudrate: int, parity: str, address: int):
        parity_map = {
            "NONE": serial.PARITY_NONE,
            "EVEN": serial.PARITY_EVEN,
            "ODD": serial.PARITY_ODD,
        }

        self.address = address

        self.serial = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=parity_map[parity],
            stopbits=serial.STOPBITS_ONE,
            timeout=0.3,
            write_timeout=0.3,
        )

    def close(self) -> None:
        self.serial.close()

    def _transfer(self, request: bytes, expected: int) -> bytes:
        frame = request + crc16(request).to_bytes(2, "little")

        self.serial.reset_input_buffer()
        self.serial.write(frame)

        response = self.serial.read(expected)

        if len(response) < 5:
            raise ModbusError("нет ответа от устройства")

        if crc16(response[:-2]) != int.from_bytes(response[-2:], "little"):
            raise ModbusError("ошибка CRC")

        if response[1] & 0x80:
            raise ModbusError(f"устройство отклонило запрос (код {response[2]})")

        return response

    def read_registers(self, function: int, start: int, count: int) -> list:
        request = bytes([self.address, function]) + \
            start.to_bytes(2, "big") + count.to_bytes(2, "big")

        response = self._transfer(request, 5 + count * 2)

        payload = response[3:3 + count * 2]

        return [int.from_bytes(payload[i:i + 2], "big")
                for i in range(0, len(payload), 2)]

    def write_register(self, address: int, value: int) -> None:
        request = bytes([self.address, 0x06]) + \
            address.to_bytes(2, "big") + value.to_bytes(2, "big")

        self._transfer(request, 8)


class MainWindow(QtWidgets.QWidget):
    POLL_INTERVAL_MS = 500

    def __init__(self):
        super().__init__()

        self.client = None

        self.setWindowTitle("Управление тиристорным контроллером")

        self._build_ui()

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.poll)

    # ------------------------------------------------------------------ UI

    def _build_ui(self) -> None:
        layout = QtWidgets.QVBoxLayout(self)

        layout.addWidget(self._build_connection_box())
        layout.addWidget(self._build_status_box())
        layout.addWidget(self._build_control_box())
        layout.addWidget(self._build_settings_box())

        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.setMaximumBlockCount(200)
        layout.addWidget(self.log)

    def _build_connection_box(self) -> QtWidgets.QGroupBox:
        box = QtWidgets.QGroupBox("Подключение")
        grid = QtWidgets.QGridLayout(box)

        self.port_combo = QtWidgets.QComboBox()
        self.refresh_ports()

        self.baud_combo = QtWidgets.QComboBox()
        self.baud_combo.addItems([str(rate) for rate in BAUD_RATES])

        self.parity_combo = QtWidgets.QComboBox()
        self.parity_combo.addItems(PARITIES)

        self.address_spin = QtWidgets.QSpinBox()
        self.address_spin.setRange(1, 247)
        self.address_spin.setValue(1)

        self.connect_button = QtWidgets.QPushButton("Подключиться")
        self.connect_button.clicked.connect(self.toggle_connection)

        refresh_button = QtWidgets.QPushButton("Обновить порты")
        refresh_button.clicked.connect(self.refresh_ports)

        grid.addWidget(QtWidgets.QLabel("Порт"), 0, 0)
        grid.addWidget(self.port_combo, 0, 1)
        grid.addWidget(refresh_button, 0, 2)
        grid.addWidget(QtWidgets.QLabel("Скорость"), 1, 0)
        grid.addWidget(self.baud_combo, 1, 1)
        grid.addWidget(QtWidgets.QLabel("Чётность"), 2, 0)
        grid.addWidget(self.parity_combo, 2, 1)
        grid.addWidget(QtWidgets.QLabel("Адрес"), 3, 0)
        grid.addWidget(self.address_spin, 3, 1)
        grid.addWidget(self.connect_button, 3, 2)

        return box

    def _build_status_box(self) -> QtWidgets.QGroupBox:
        box = QtWidgets.QGroupBox("Состояние")
        grid = QtWidgets.QGridLayout(box)

        self.status_labels = {}

        fields = [
            ("state", "Состояние"),
            ("sync", "Синхронизация"),
            ("frequency", "Частота, Гц"),
            ("angle", "Угол, град"),
            ("elapsed", "Время работы, мс"),
            ("zero_cross", "Переходы через ноль"),
            ("glitch", "Отбраковано"),
            ("ch1", "Импульсы CH1"),
            ("ch2", "Импульсы CH2"),
            ("watchdog", "Сторож CH2"),
        ]

        for row, (key, title) in enumerate(fields):
            label = QtWidgets.QLabel("-")

            grid.addWidget(QtWidgets.QLabel(title), row % 5, (row // 5) * 2)
            grid.addWidget(label, row % 5, (row // 5) * 2 + 1)

            self.status_labels[key] = label

        return box

    def _build_control_box(self) -> QtWidgets.QGroupBox:
        box = QtWidgets.QGroupBox("Управление")
        layout = QtWidgets.QHBoxLayout(box)

        self.angle_spin = QtWidgets.QSpinBox()
        self.angle_spin.setRange(5, 175)
        self.angle_spin.setValue(90)

        angle_button = QtWidgets.QPushButton("Задать угол")
        angle_button.clicked.connect(
            lambda: self.write(HOLDING_ANGLE, self.angle_spin.value()))

        start_button = QtWidgets.QPushButton("START")
        start_button.clicked.connect(
            lambda: self.write(HOLDING_COMMAND, CMD_START))

        stop_button = QtWidgets.QPushButton("STOP")
        stop_button.clicked.connect(
            lambda: self.write(HOLDING_COMMAND, CMD_STOP))

        save_button = QtWidgets.QPushButton("Сохранить настройки")
        save_button.clicked.connect(
            lambda: self.write(HOLDING_COMMAND, CMD_SAVE))

        layout.addWidget(QtWidgets.QLabel("Угол"))
        layout.addWidget(self.angle_spin)
        layout.addWidget(angle_button)
        layout.addStretch()
        layout.addWidget(start_button)
        layout.addWidget(stop_button)
        layout.addWidget(save_button)

        return box

    def _build_settings_box(self) -> QtWidgets.QGroupBox:
        box = QtWidgets.QGroupBox("Настройки")
        grid = QtWidgets.QGridLayout(box)

        self.pulse_spin = QtWidgets.QSpinBox()
        self.pulse_spin.setRange(50, 500)
        self.pulse_spin.setSingleStep(10)

        self.mode_combo = QtWidgets.QComboBox()
        self.mode_combo.addItems(WORK_MODES)

        self.duration_spin = QtWidgets.QSpinBox()
        self.duration_spin.setRange(100, 1000)
        self.duration_spin.setSingleStep(100)

        self.max_run_spin = QtWidgets.QSpinBox()
        self.max_run_spin.setRange(1, 120)

        self.freq_dev_spin = QtWidgets.QSpinBox()
        self.freq_dev_spin.setRange(5, 100)
        self.freq_dev_spin.setSingleStep(5)

        self.control_combo = QtWidgets.QComboBox()
        self.control_combo.addItems(CONTROL_MODES)

        rows = [
            ("Импульс, мкс", self.pulse_spin, HOLDING_PULSE_US),
            ("Режим работы", self.mode_combo, HOLDING_WORK_MODE),
            ("Время, мс", self.duration_spin, HOLDING_DURATION_MS),
            ("Аварийный таймаут, с", self.max_run_spin, HOLDING_MAX_RUN_S),
            ("Допуск частоты, 0.1 Гц", self.freq_dev_spin, HOLDING_FREQ_DEV),
            ("Режим управления", self.control_combo, HOLDING_CONTROL_MODE),
        ]

        for row, (title, widget, register) in enumerate(rows):
            button = QtWidgets.QPushButton("Записать")
            button.clicked.connect(
                lambda _, w=widget, r=register: self.write(r, self.value_of(w)))

            grid.addWidget(QtWidgets.QLabel(title), row, 0)
            grid.addWidget(widget, row, 1)
            grid.addWidget(button, row, 2)

        read_button = QtWidgets.QPushButton("Прочитать настройки из устройства")
        read_button.clicked.connect(self.read_settings)

        grid.addWidget(read_button, len(rows), 0, 1, 3)

        return box

    @staticmethod
    def value_of(widget) -> int:
        if isinstance(widget, QtWidgets.QComboBox):
            return widget.currentIndex()

        return widget.value()

    # ------------------------------------------------------------- actions

    def refresh_ports(self) -> None:
        current = self.port_combo.currentText() if hasattr(self, "port_combo") else ""

        self.port_combo.clear()
        self.port_combo.addItems(
            [port.device for port in serial.tools.list_ports.comports()])

        index = self.port_combo.findText(current)

        if index >= 0:
            self.port_combo.setCurrentIndex(index)

    def toggle_connection(self) -> None:
        if self.client is not None:
            self.timer.stop()
            self.client.close()
            self.client = None

            self.connect_button.setText("Подключиться")
            self.message("Отключено")
            return

        if not self.port_combo.currentText():
            self.message("Не выбран последовательный порт")
            return

        try:
            self.client = ModbusClient(
                port=self.port_combo.currentText(),
                baudrate=int(self.baud_combo.currentText()),
                parity=self.parity_combo.currentText(),
                address=self.address_spin.value(),
            )
        except (serial.SerialException, OSError) as error:
            self.message(f"Не удалось открыть порт: {error}")
            return

        self.connect_button.setText("Отключиться")
        self.message("Подключено")

        self.read_settings()
        self.timer.start(self.POLL_INTERVAL_MS)

    def write(self, register: int, value: int) -> None:
        if self.client is None:
            self.message("Нет подключения")
            return

        try:
            self.client.write_register(register, value)
        except (ModbusError, serial.SerialException) as error:
            self.message(f"Ошибка записи: {error}")
            return

        self.message(f"Регистр {register} = {value}")

    def read_settings(self) -> None:
        if self.client is None:
            return

        try:
            values = self.client.read_registers(0x03, 0, HOLDING_COUNT)
        except (ModbusError, serial.SerialException) as error:
            self.message(f"Ошибка чтения настроек: {error}")
            return

        self.angle_spin.setValue(values[HOLDING_ANGLE])
        self.pulse_spin.setValue(values[HOLDING_PULSE_US])
        self.mode_combo.setCurrentIndex(values[HOLDING_WORK_MODE])
        self.duration_spin.setValue(values[HOLDING_DURATION_MS])
        self.max_run_spin.setValue(values[HOLDING_MAX_RUN_S])
        self.freq_dev_spin.setValue(values[HOLDING_FREQ_DEV])
        self.control_combo.setCurrentIndex(values[HOLDING_CONTROL_MODE])

    def poll(self) -> None:
        if self.client is None:
            return

        try:
            values = self.client.read_registers(0x04, 0, INPUT_COUNT)
        except (ModbusError, serial.SerialException) as error:
            self.message(f"Ошибка опроса: {error}")
            return

        state = values[0]

        self.status_labels["state"].setText(
            DEVICE_STATES[state] if state < len(DEVICE_STATES) else str(state))
        self.status_labels["sync"].setText("есть" if values[1] else "НЕТ")
        self.status_labels["frequency"].setText(f"{values[2] / 10:.1f}")
        self.status_labels["angle"].setText(str(values[4]))
        self.status_labels["elapsed"].setText(str(values[5]))
        self.status_labels["zero_cross"].setText(str(values[6]))
        self.status_labels["glitch"].setText(str(values[7]))
        self.status_labels["ch1"].setText(str(values[8]))
        self.status_labels["ch2"].setText(str(values[9]))
        self.status_labels["watchdog"].setText(str(values[10]))

    def message(self, text: str) -> None:
        self.log.appendPlainText(text)


def main() -> int:
    app = QtWidgets.QApplication(sys.argv)

    window = MainWindow()
    window.resize(720, 640)
    window.show()

    return app.exec_()


if __name__ == "__main__":
    sys.exit(main())
