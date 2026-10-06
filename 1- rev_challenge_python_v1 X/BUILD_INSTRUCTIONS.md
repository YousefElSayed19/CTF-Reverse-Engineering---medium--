# تعليمات بناء تحدي Reverse Engineering (game.exe + checker.exe)

## المتطلبات (على جهاز Windows بتاعك)

```powershell
pip install pyinstaller cryptography
```

## ترتيب الخطوات

### 1. (اختياري) خصص القيم السرية قبل البناء

- في `src/logic.py`: غيّر `REAL_FLAG` للفلاج الحقيقي بتاعك.
- في `src/game.py`: غيّر `REAL_PASSWORD_1` و `REAL_PASSWORD_2` و `XOR_KEY` و
  `PASSWORD_2_XOR_KEY` لو عايز قيم مختلفة.
- في `src/build_encrypt.py`: تأكد إن `PASSWORD_2` فيه **نفس القيمة بالظبط**
  اللي في `REAL_PASSWORD_2` جوا `game.py`.
- في `src/game.py` → `QUIZ`: لو غيرت الـ XOR key أو اسم الـ function،
  لازم تحدّث الـ `answer_hash` بتاع كل سؤال كمان (استخدم
  `hashlib.sha256(b"NEW_ANSWER").hexdigest()` عشان تطلع الهاش الجديد).

### 2. شفّر `logic.py` وحط الناتج جوا `checker.py`

```powershell
cd src
python build_encrypt.py
```

ده هيطلعلك سطر زي:
```python
ENCRYPTED_LOGIC = b'gAAAAABk....'
```

افتح `checker.py` واستبدل السطر:
```python
ENCRYPTED_LOGIC = b"PASTE_ENCRYPTED_BYTES_HERE"
```
بالسطر اللي طلع لك.

### 3. ابني الملفين كـ exe مستقلين (onefile)

```powershell
pyinstaller --onefile --console --name game game.py
pyinstaller --onefile --console --name checker checker.py
```

الملفات الناتجة هتكون في `dist/game.exe` و `dist/checker.exe`.
الملفين دول **مستقلين تمامًا** - أي حد يشغلهم على ويندوز من غير ما يسطب
Python أو أي مكتبة.

### 4. (مهم جدًا) امسح ملفات المصدر السرية قبل ما توزع أي حاجة

**لا توزع أبدًا** الملفات دي مع التحدي:
- `logic.py` (الكود الأصلي قبل التشفير)
- `build_encrypt.py`
- `encrypted_logic.txt`
- أي نسخة من `game.py` / `checker.py` قبل ما تتعمل compile (الـ .py نفسها،
  بس الـ .exe الناتج)

**اللي يتوزع للاعب فعليًا:**
- `dist/game.exe`
- `dist/checker.exe`

### 5. اختبار نهائي قبل التسليم

شغّل الملفين بنفسك من الصفر وجرب:
- باسورد غلط في game.exe → لازم يترفض
- باسورد صح + إجابات quiz غلط → لازم يترفض
- باسورد صح + quiz صح + تخسر المينی-جيم → لازم يترفض
- المسار الكامل الصح → لازم يطلعلك Password 2
- checker.exe بباسورد غلط → لازم يرفض من غير ما يفك أي حاجة
- checker.exe بباسورد صح + فلاج غلط → يرفض الفلاج بس يكون فاك التشفير
- checker.exe بباسورد صح + فلاج صح → رسالة النجاح

## ملاحظة أمان

الطريقة دي (تشفير الكود بمفتاح = الباسورد، وتنفيذه بـ `exec()` وقت
التشغيل فقط) بتضمن إن أي حد يحاول يعمل static analysis لـ checker.exe من
غير الباسورد الصح، هيلاقي بس bytes مشفرة (ciphertext) مش كود حقيقي قابل
للقراءة أو التنفيذ. اللاعب الوحيد اللي يقدر "يشغّل" منطق التحقق الحقيقي هو
اللي فعلاً عدى بالترتيب الصحيح من game.exe.
