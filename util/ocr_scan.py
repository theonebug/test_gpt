# -*- coding: utf-8 -*-
"""Распознавание сканов руководства старого регулятора через RapidOCR."""
import os
import sys

try:
    from rapidocr_onnxruntime import RapidOCR
except Exception as exc:  # noqa: BLE001
    sys.exit("RapidOCR не установлен: %s" % exc)

BASE = os.path.dirname(os.path.abspath(__file__))
IMAGES = [os.path.join(BASE, f) for f in ("1.jpg", "2.jpg", "3.jpg", "4.jpg")]
OUT = os.path.join(BASE, "scan_text.txt")


def main() -> int:
    engine = RapidOCR()
    with open(OUT, "w", encoding="utf-8") as out:
        for img in IMAGES:
            out.write("\n" + "=" * 60 + "\n")
            out.write("ФАЙЛ: %s\n" % os.path.basename(img))
            out.write("=" * 60 + "\n")
            if not os.path.exists(img):
                out.write("[файл не найден]\n")
                continue
            result, _ = engine(img)
            if not result:
                out.write("[текст не распознан]\n")
                continue
            # RapidOCR возвращает строки в порядке сканирования.
            # Для простоты выводим строки сверху вниз по координате Y.
            rows = sorted(result, key=lambda r: (r[0][0][1], r[0][0][0]))
            for box, text, score in rows:
                y = int(box[0][1])
                out.write("[y=%04d] %s\n" % (y, text))
    print("Готово, результат в:", OUT)
    return 0


if __name__ == "__main__":
    sys.exit(main())