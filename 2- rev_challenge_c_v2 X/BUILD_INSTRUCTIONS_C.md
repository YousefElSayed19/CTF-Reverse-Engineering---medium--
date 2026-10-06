# تعليمات بناء تحدي Reverse Engineering (نسخة C) على Windows

## 1. ثبّت MinGW (مترجم C لويندوز)

لو مش مسطب عندك `gcc` على ويندوز، أسهل طريقة:

- حمّل **MSYS2** من: https://www.msys2.org/
- بعد التسطيب، افتح "MSYS2 MINGW64" من قائمة Start
- فيه اكتب:
  ```
  pacman -S mingw-w64-x86_64-gcc
  ```
- بعد ما يخلص، ضيف المسار ده لمتغير PATH بتاع ويندوز (System Environment Variables):
  ```
  C:\msys64\mingw64\bin
  ```
- قفل كل الـ cmd المفتوحة وافتح واحدة جديدة، وجرب:
  ```
  gcc --version
  ```
  لازم يطلعلك رقم نسخة.

## 2. الملفات اللي هتستخدمها

من فولدر `src/`:
- `game.c`, `checker.c` — الملفين الأساسيين
- `aes.c`, `aes.h` — تطبيق AES-128 (مُختبر ومتأكد منه ضد NIST test vectors)
- `base64.c`, `base64.h` — دوال base64 encode/decode
- `gen_values.c`, `gen_flag.c` — أدوات مساعدة (DEV ONLY) تستخدمها بس لو عايز تغيّر الباسوردات أو الفلاج

## 3. ابني الملفين النهائيين

من جوا فولدر `src`:

```
gcc -o game.exe game.c aes.c base64.c -Wall
gcc -o checker.exe checker.c aes.c -Wall
```

لو خلصوا من غير errors، هتلاقي `game.exe` و `checker.exe` في نفس الفولدر.

## 4. جرب الملفين

### game.exe
```
game.exe
```
- Password: `r3v3rs3_m3_pl2`
- سؤال 1: `4B`
- سؤال 2: `XOR`
- سؤال 3: `verify_sys_integrity`
- تخمين الرقم: `7`
- هيطلعلك Password 2: `ch3ck3r_unl0ck_key`

### checker.exe
```
checker.exe
```
- Enter unlock password: `ch3ck3r_unl0ck_key`
- هيطلعلك الفلاج: `duck{4yBlGzSTp+5sP4Q3!eG#fE$gHGYjkpQQ}`

## 5. لو عايز تغيّر الفلاج أو الباسوردات

### أ. غيّر القيم في الأدوات المساعدة

- في `gen_values.c`: غيّر `password1` (الباسورد الأول)، أو `password2` (في المتغير داخل نفس الملف) حسب الأسماء الموجودة
- في `gen_flag.c`: غيّر `flag` للفلاج الجديد، وتأكد إن `password2` هنا **مطابق تمامًا** للي في `gen_values.c`

### ب. ابني الأدوات وشغّلها لتوليد القيم الجديدة

```
gcc -o gen_values.exe gen_values.c aes.c base64.c -Wall
gen_values.exe
```
هيطلعلك سطرين، انسخهم وحطهم في `game.c` بدل القيم الحالية (`ENCODED_PASSWORD_1` و `P2_CIPHERTEXT`).

```
gcc -o gen_flag.exe gen_flag.c aes.c -Wall
gen_flag.exe
```
هيطلعلك `FLAG_CIPHERTEXT` الجديد، حطه في `checker.c` بدل القديم.

### ج. أعد بناء game.exe و checker.exe بالأوامر في خطوة 3

## 6. (مهم) قبل ما توزع أي حاجة للاعبين

- متوزعش ملفات `.c` ولا `gen_values.exe` / `gen_flag.exe` خالص
- وزّع بس `game.exe` و `checker.exe` الناتجين
- جرب الملفين بنفسك من الصفر قبل التسليم للتأكد

## ملاحظة أمان (للـ write-up بتاعك)

- الـ AES-128 implementation اتبنى من الصفر (single-file، public algorithm) واتأكد ضد NIST official test vectors رسميًا، فهو تشفير حقيقي مش تمثيل شكلي
- مفتاح AES بيتاخد من كل حروف الباسورد (مش بس أول 16 حرف) عشان نمنع أي collision بين باسوردات قريبة من بعض
- الـ IV مش سري (ده طبيعي في AES-CBC، السرية في الـ key بس)
