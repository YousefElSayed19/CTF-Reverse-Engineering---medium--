# تعليمات بناء تحدي Reverse Engineering (نسخة C - v2 Medium)

## إيه اللي اتغيّر عن النسخة الأولى (v1)؟

النسخة دي فيها 3 طبقات صعوبة إضافية على `game.c` (الباسورد الأول):

1. **Multi-byte repeating-key XOR** بدل مفتاح XOR واحد ثابت - المفتاح الحقيقي
   دلوقتي 4 بايتات (`4B 33 79 21`) بتتكرر بالتبادل على طول الباسورد، مش بايت
   واحد ثابت زي الأول.

2. **إخفاء الـ base64 string من `strings`** - القيمة المشفرة متخزنة في الملف
   بعد ما كل بايت فيها اتعمله XOR إضافي بقيمة ثابتة (`0x01`)، فمش هتظهر
   كنص base64 عادي لو حد شغّل أداة `strings` على الملف - محتاج يروح
   يفك التشفير الإضافي ده الأول من جوا Ghidra.

3. **Anti-debugging check** - لو حد حاول يعمل attach بـ debugger (زي
   x64dbg أو x32dbg) أثناء تشغيل `game.exe`، دالة التحقق من الباسورد
   هترفض **أي** باسورد (حتى الصحيح) من غير ما تدي أي رسالة مختلفة
   تفضحها. ده بيجبر اللاعب يعتمد على التحليل الساكن (static analysis
   في Ghidra) بدل ما يحاول يشغّل البرنامج جوا debugger مباشرة.

`checker.c` فضل **زي ما هو من غير تغيير** (لسه AES-128-CBC حقيقي).

## القيم للاختبار (نفس القيم القديمة، المنطق بس اللي اتغيّر)

- Password 1: `r3v3rs3_m3_pl2`
- أسئلة الكويز الجديدة:
  - عدد بايتات مفتاح الـ XOR: `4`
  - أول بايت في المفتاح (hex): `4B`
  - اسم الـ function: `verify_sys_integrity`
- تخمين الرقم: `7`
- Password 2: `ch3ck3r_unl0ck_key`
- الفلاج: `duck{4yBlGzSTp+5sP4Q3!eG#fE$gHGYjkpQQ}`

## خطوات البناء (MinGW/TDM-GCC - زي v1 بالظبط)

```
gcc -o game.exe game.c aes.c base64.c -Wall
gcc -o checker.exe checker.c aes.c -Wall
```

## اختبار الـ anti-debugging فعليًا على جهازك

بعد ما تبني `game.exe`:

### 1. تأكد إن التشغيل العادي لسه شغال

```
game.exe
```
اتبع نفس القيم فوق، المفروض يعدي عادي ويطلّع Password 2.

### 2. اختبر إن الـ anti-debug بيشتغل مع debugger حقيقي

لو عندك x64dbg أو x32dbg مسطب:
1. افتح `game.exe` **جوا** x64dbg (File → Open)
2. شغّله (F9) لحد ما يطلب الباسورد
3. اكتب الباسورد **الصحيح** (`r3v3rs3_m3_pl2`) وادوس Enter
4. **المتوقع:** يقولك "Incorrect password" حتى إنك كتبت الباسورد الصح - ده
   دليل إن الـ anti-debug شغال وبيكتشف إن فيه debugger متصل

### ملاحظة فنية مهمة

- الكود بيستخدم `IsDebuggerPresent()` بتاعة Windows API مباشرة
  (`#include <windows.h>`)، وده API قياسي وموثوق 100% على ويندوز، متاح
  تلقائيًا مع أي compiler بما فيهم MinGW/TDM-GCC من غير أي تسطيب إضافي
- الكود بيتأكد أول حاجة في `verify_sys_integrity()` قبل أي عملية فك
  تشفير، فمفيش أي استهلاك إضافي أو تأخير ملحوظ في التشغيل العادي

## لو عايز تغيّر القيم

### الباسورد الأول / مفتاح XOR

عدّل في `gen_values.c`: غيّر `password1` و/أو `XOR_KEY`، ثم:
```
gcc -o gen_values.exe gen_values.c base64.c -Wall
gen_values.exe
```
انسخ قيمة `MASKED_B64` الجديدة والصقها في `game.c` (وحدّث `MASKED_B64_LEN`
و`XOR_KEY` في `game.c` لو غيرته).

**لو غيرت `XOR_KEY`**: لازم تحدّث أسئلة الكويز في `run_quiz()` بالقيم
الجديدة (عدد البايتات، وأول بايت في المفتاح).

### الفلاج / الباسورد الثاني

زي ما هو موضح في `BUILD_INSTRUCTIONS_C.md` الأصلي (استخدام `gen_flag.c`).

## (مهم) قبل التوزيع

متوزعش: `gen_values.c`, `gen_flag.c`, `test_antidebug.c` ولا أي ملف `.c`
خالص. وزّع بس `game.exe` و `checker.exe` الناتجين.
