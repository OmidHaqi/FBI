<div dir='rtl'>

# ForceBindIP - ابزار کنترل Interface شبکه

## چیکار می‌کنه؟
ForceBindIP هر برنامه‌ای رو مجبور می‌کنه از interface یا IP خاصی برای اتصالات شبکه استفاده کنه. به جای اینکه برنامه‌ها خودشون تصمیم بگیرن از کدوم کارت شبکه استفاده کنن، این ابزار تماس‌های شبکه رو رهگیری می‌کنه و از طریق interface مورد نظر شما هدایت می‌کنه.

## چرا استفاده کنیم؟
- **مسیریابی VPN**: برخی برنامه‌ها از VPN و بقیه از اتصال عادی استفاده کنن
- **تنظیمات چند شبکه**: انتخاب کارت شبکه برای برنامه‌ها (WiFi، کابل، و غیره)
- **تست**: آزمایش برنامه‌ها با تنظیمات مختلف شبکه
- **امنیت**: اطمینان از اینکه برنامه‌های حساس فقط از interface امن استفاده کنن
- **مدیریت پهنای باند**: هدایت دانلودهای سنگین از طریق اتصالات خاص

## قابلیت‌ها
-  **چند پلتفرمه**: کار می‌کنه روی Windows (DLL injection) و Linux (LD_PRELOAD)
-  **IPv4 و IPv6**: پشتیبانی کامل از هر دو نسخه IP
-  **Binding Interface**: اتصال با نام interface یا آدرس IP
-  **پشتیبانی پروتکل**: socket binding برای TCP و UDP
-  **Real-time**: نیازی به ریستارت برنامه نیست - با برنامه‌های در حال اجرا کار می‌کنه
-  **Logging جزئی**: اطلاعات دقیق اتصال برای عیب‌یابی

## شروع سریع

### Linux
```bash
# ساخت
make clean && make

# لیست interface های موجود
./forcebindip -l

# مجبور کردن curl به استفاده از IP خاص
./forcebindip -4 192.168.1.100 curl http://httpbin.org/ip

# مجبور کردن برنامه از طریق VPN
./forcebindip -i tun0 firefox

# تست با برنامه نمونه
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

### Windows
```cmd
# لیست interface های شبکه
injector.exe -l

# مجبور کردن برنامه به استفاده از IP خاص
injector.exe -4 192.168.1.1 program.exe

# مجبور کردن برنامه از طریق GUID interface
injector.exe -i 11111111-2222-3333-4444-555555555555 program.exe
```

## گزینه‌های دستور

### Linux
```bash
./forcebindip [options] <command> [args...]

-4 <ip>      # استفاده از آدرس IPv4 مشخص
-6 <ip>      # استفاده از آدرس IPv6 مشخص  
-i <iface>   # استفاده از interface شبکه (eth0, wlo1, tun0, و غیره)
-v           # خروجی پرجزئیات (نمایش جزئیات اتصال)
-l           # لیست تمام interface های موجود
-h           # نمایش راهنما
```

### Windows
```cmd
injector.exe [options] <program> [args...]

-4 <ip>      # استفاده از آدرس IPv4 مشخص
-6 <ip>      # استفاده از آدرس IPv6 مشخص
-i <GUID>    # استفاده از interface شبکه با GUID
-v           # خروجی پرجزئیات
-l           # لیست interface های شبکه با GUID
-t           # فقط TCP socket ها رو bind کن
-u           # فقط UDP socket ها رو bind کن
-p <port>    # bind به پورت خاص
```

## مثال‌های کاربردی

### مجبور کردن برنامه‌های خاص
```bash
# مثال‌های Linux
./forcebindip -i eth0 wget http://example.com          # دانلود از طریق کابل شبکه
./forcebindip -i wlo1 curl http://api.service.com      # تماس API از طریق WiFi
./forcebindip -i tun0 ssh user@server.com             # SSH از طریق VPN
./forcebindip -4 10.0.0.5 firefox                     # مرورگر از طریق IP خاص

# مثال‌های Windows
injector.exe -i {GUID} chrome.exe                     # Chrome از طریق آداپتر خاص
injector.exe -4 192.168.1.100 uTorrent.exe           # تورنت از طریق IP خاص
injector.exe -i {VPN-GUID} discord.exe                # Discord از طریق VPN
```

### تست و بررسی
```bash
# چک کردن IP واقعی قبل از تغییر
curl http://httpbin.org/ip

# مجبور کردن از طریق interface متفاوت
./forcebindip -4 192.168.1.100 curl http://httpbin.org/ip

# مقایسه IP ها - باید متفاوت باشن!
```

### موارد استفاده VPN
```bash
# مجبور کردن برنامه‌های حساس فقط از طریق VPN
./forcebindip -i tun0 telegram-desktop
./forcebindip -i tun0 signal-desktop
./forcebindip -i tun0 firefox --private-window

# نگه داشتن برنامه‌های عادی روی اتصال معمولی
./forcebindip -i wlo1 spotify
./forcebindip -i eth0 steam
```

## چطور کار می‌کنه؟

### Linux (روش LD_PRELOAD)
1. تماس‌های سیستمی socket رو با `LD_PRELOAD` رهگیری می‌کنه
2. وقتی برنامه socket ایجاد می‌کنه، ForceBindIP اونو به interface انتخابی شما bind می‌کنه
3. تمام ترافیک شبکه از اون برنامه از طریق interface مشخص شده می‌ره
4. کار می‌کنه با: `socket()`, `bind()`, `connect()`, `sendto()`, `getsockname()`

### Windows (روش DLL Injection)
1. یک DLL رو به پروسه هدف تزریق می‌کنه
2. توابع WinSock API رو با کتابخانه MinHook هوک می‌کنه
3. تماس‌های شبکه رو به استفاده از interface/IP مشخص هدایت می‌کنه
4. کار می‌کنه با: `socket`, `bind`, `connect`, `WSASendTo`, `getsockname`

## برنامه‌های پشتیبانی شده
- **مرورگرهای وب**: Firefox, Chrome, Edge
- **ارتباطات**: Discord, Telegram, Signal, WhatsApp
- **توسعه**: curl, wget, ssh, git, npm, pip
- **سرگرمی**: Steam, Spotify, VLC
- **به اشتراک‌گذاری فایل**: کلاینت‌های BitTorrent, کلاینت‌های FTP
- **برنامه‌های سفارشی**: هر برنامه‌ای که از socket API های استاندارد استفاده کنه

## نصب

### Linux
```bash
# کلون و ساخت
git clone <repository-url>
cd FBI
make clean && make

# فایل‌های ایجاد شده:
# - forcebindip: اجرایی اصلی
# - libforcebindip.so: کتابخانه هوک
# - test_ipv4, test_ipv6: برنامه‌های تست
```

### Windows
```bash
# از Visual Studio 2022 استفاده کنید
# فایل‌های solution رو در پوشه‌های dll/ و injector/ باز کنید
# با وابستگی کتابخانه MinHook build کنید
```

## عیب‌یابی

### مشکلات متداول

**خطای "Invalid argument":**
```bash
# ❌ از loopback برای اتصالات خارجی استفاده نکنید
./forcebindip -4 127.0.0.1 curl http://google.com

# ✅ از IP واقعی interface استفاده کنید
./forcebindip -4 192.168.1.3 curl http://google.com
```

**دسترسی غیرمجاز:**
```bash
# برخی برنامه‌ها نیاز به دسترسی بالا دارن
sudo ./forcebindip -i eth0 ping google.com
```

**کتابخانه پیدا نشد:**
```bash
# مطمئن بشید کتابخانه وجود داره
ls -la libforcebindip.so

# در صورت نیاز دوباره بسازید
make clean && make
```

### دیباگ کردن
```bash
# حالت پرجزئیات رو فعال کنید تا ببینید چه اتفاقی می‌افته
./forcebindip -v -4 192.168.1.3 curl http://httpbin.org/ip

# interface های موجود رو چک کنید
./forcebindip -l

# اتصال رو دستی تست کنید
ping -I 192.168.1.3 google.com
```

## استفاده پیشرفته

### تنظیم چندین Interface
```bash
# مسیریابی سرویس‌های مختلف از طریق interface های مختلف
./forcebindip -i eth0 ./download_script.sh     # دانلودهای سنگین از طریق کابل
./forcebindip -i wlo1 ./api_client.py          # تماس‌های API از طریق WiFi
./forcebindip -i tun0 ./secure_chat.sh         # چت امن از طریق VPN
```

### توسعه و تست
```bash
# تست IPv4 در مقابل IPv6
./forcebindip -4 192.168.1.3 ./network_test
./forcebindip -6 2001:db8::1 ./network_test

# تست پروتکل‌های مختلف
./forcebindip -v -4 192.168.1.3 nc -u 8.8.8.8 53    # UDP
./forcebindip -v -4 192.168.1.3 nc 8.8.8.8 80       # TCP
```

## فعال/غیرفعال کردن

### فعال کردن ForceBindIP
```bash
# برای هر برنامه جداگانه
./forcebindip -i tun0 firefox &          # فایرفاکس فقط از VPN
./forcebindip -i eth0 steam &            # Steam از کابل شبکه

# برای session کامل (متغیر محیطی)
export LD_PRELOAD="$PWD/libforcebindip.so"
export FORCEBINDIP_IPV4="192.168.1.3"
# حالا همه برنامه‌ها از IP مشخص شده استفاده می‌کنن
```

### غیرفعال کردن ForceBindIP
```bash
# Linux - فقط برنامه رو بدون ForceBindIP اجرا کنید
./my_program                             # بدون ForceBindIP

# یا متغیر محیطی رو پاک کنید
unset LD_PRELOAD
unset FORCEBINDIP_IPV4

# Windows - برنامه رو بدون injector اجرا کنید
program.exe                              # بدون injector
```

### تست فعال/غیرفعال بودن
```bash
# تست بدون ForceBindIP
curl http://httpbin.org/ip

# تست با ForceBindIP
./forcebindip -4 192.168.1.100 curl http://httpbin.org/ip

# IP ها باید متفاوت باشن
```

## ساختار فایل‌ها
```
FBI/
├── build/                     # پوشه خروجی ساخت
├── common/                    # Header های چند پلتفرمه
├── dll/                       # کد منبع DLL ویندوز
├── forcebindip                # اجرایی اصلی Linux
├── injector/                  # کد منبع injector ویندوز
├── injector_crossplatform/    # Injector چند پلتفرمه
├── libforcebindip_simple.c    # کد منبع کتابخانه هوک Linux (C)
├── libforcebindip.so          # کتابخانه هوک Linux (کامپایل شده)
├── Makefile                   # سیستم ساخت Linux
├── README_FA.md               # مستندات فارسی
├── README.md                  # مستندات انگلیسی
├── test_crossplatform/        # برنامه‌های تست چند پلتفرمه
├── test_ipv4                  # اجرایی تست IPv4
├── test_ipv6                  # اجرایی تست IPv6
├── testv4/                    # کد منبع تست IPv4 اصلی
└── testv6/                    # کد منبع تست IPv6 اصلی
```

## راهنمای کامل استفاده

### جزئیات نصب

#### نیازمندی‌های ساخت در Linux
```bash
# اطمینان از وجود پکیج‌های مورد نیاز
sudo apt-get update
sudo apt-get install build-essential gcc g++ make

# کلون و ساخت
git clone <repository-url>
cd FBI
make clean && make

# فایل‌های ایجاد شده:
# - forcebindip: اجرایی اصلی injector
# - libforcebindip.so: کتابخانه هوک برای LD_PRELOAD
# - test_ipv4, test_ipv6: برنامه‌های تست
```

#### نیازمندی‌های ساخت در Windows
- سیستم‌عامل Windows با پشتیبانی Winsock
- Visual Studio 2022
- کتابخانه MinHook برای هوک کردن توابع Winsock

### مرجع کامل دستورات

#### تمام گزینه‌های Linux
```bash
./forcebindip [options] <command> [args...]

-4 <ip>        # Bind به آدرس IPv4 مشخص
-6 <ip>        # Bind به آدرس IPv6 مشخص  
-i <interface> # Bind به interface شبکه
-p <port>      # Bind به پورت خاص (فقط Linux)
-v             # فعال کردن خروجی پرجزئیات
-l             # لیست interface های موجود
-h, --help     # نمایش راهنما
```

#### تمام گزینه‌های Windows
```cmd
injector.exe [options] <program> [args...]

-4 <IPv4>      # Bind به آدرس IPv4 مشخص
-6 <IPv6>      # Bind به آدرس IPv6 مشخص
-i <GUID>      # Bind به interface شبکه با GUID
-v             # فعال کردن خروجی پرجزئیات
-o <file>      # ذخیره log های DLL در فایل مشخص
-l             # لیست interface های موجود با GUID
-t             # Bind فقط TCP socket ها (پیش‌فرض اگر binding مشخص نشده)
-u             # Bind فقط UDP socket ها (پیش‌فرض اگر binding مشخص نشده)
-p <port>      # Bind به پورت مشخص (نیاز به -t یا -u)
```

### مثال‌های جامع

#### عملیات پایه Interface
```bash
# لیست تمام interface های موجود
./forcebindip -l

# نمونه خروجی:
# Available Network Interfaces:
# ========================================
# Interface Name: wlo1
# Interface ID: wlo1
# Description: Linux Network Interface
# Status: Up
# IPv4 Address: 192.168.1.3
```

#### مثال‌های IPv4 Binding
```bash
# مجبور کردن curl به استفاده از آدرس IP خاص
./forcebindip -v -4 192.168.1.100 curl http://httpbin.org/ip

# مجبور کردن wget به استفاده از IP خاص
./forcebindip -4 10.0.0.5 wget http://example.com

# تست با برنامه نمونه
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

#### مثال‌های Interface Binding
```bash
# مجبور کردن اتصال از طریق interface خاص
./forcebindip -v -i wlo1 curl http://httpbin.org/ip

# استفاده از interface کابلی
./forcebindip -i eth0 wget http://example.com

# تست با interface binding
./forcebindip -v -i wlo1 ./test_ipv4
```

#### مثال‌های پشتیبانی IPv6
```bash
# Bind به آدرس IPv6 (در صورت موجود بودن)
./forcebindip -6 2001:db8::1 curl -6 http://httpbin.org/ip

# استفاده از interface IPv6
./forcebindip -i eth0 curl -6 http://example.com
```

#### مثال‌های استفاده پیشرفته
```bash
# حالت پرجزئیات برای عیب‌یابی
./forcebindip -v -4 192.168.1.3 ssh user@server.com

# مجبور کردن برنامه خاص از طریق interface VPN
./forcebindip -i tun0 firefox

# تست UDP binding
./forcebindip -v -4 192.168.1.3 dig @8.8.8.8 google.com
```

### جزئیات پیاده‌سازی فنی

#### پیاده‌سازی Linux (LD_PRELOAD)
- استفاده از تکنیک **LD_PRELOAD** برای رهگیری توابع socket
- Hook ها: `socket()`, `bind()`, `connect()`, `sendto()`, `getsockname()`
- **SO_BINDTODEVICE** برای binding به interface
- کشف interface شبکه از طریق `getifaddrs()`

#### پیاده‌سازی Windows (DLL Injection)
- استفاده از **DLL injection** با کتابخانه MinHook
- Hook کردن توابع WinSock: `socket()`, `bind()`, `connect()`, `WSASendTo()`
- Interface binding از طریق تحلیل آدرس IP

### برنامه‌های پشتیبانی شده
این ابزار با هر برنامه dynamically linked که از توابع socket استاندارد استفاده می‌کند کار می‌کند:
- **مرورگرهای وب**: Firefox, Chrome (با برخی محدودیت‌ها)
- **ابزارهای خط فرمان**: curl, wget, ssh, nc, dig
- **ابزارهای شبکه**: ping, traceroute, nmap
- **برنامه‌های سفارشی**: هر برنامه‌ای که از socket API های استاندارد استفاده کند

### راهنمای کامل عیب‌یابی

#### مشکلات متداول و راه‌حل‌ها

**۱. خطای "Invalid argument"**
معمولاً زمانی رخ می‌دهد که سعی می‌کنید آدرس loopback (127.0.0.1) را به اتصالات خارجی bind کنید:
```bash
# ❌ این ممکن است شکست بخورد:
./forcebindip -4 127.0.0.1 curl http://google.com

# ✅ از IP واقعی interface استفاده کنید:
./forcebindip -4 192.168.1.3 curl http://google.com
```

**۲. دسترسی غیرمجاز**
برخی برنامه‌ها ممکن است نیاز به دسترسی‌های بالا داشته باشند:
```bash
sudo ./forcebindip -i eth0 ping google.com
```

**۳. آدرس‌های IPv6 Link-Local**
آدرس‌های IPv6 link-local نیاز به تعیین scope دارند:
```bash
# اضافه کردن %interface به آدرس‌های link-local
./forcebindip -6 fe80::1%eth0 curl -6 http://example.com
```

**۴. کتابخانه پیدا نشد**
اطمینان حاصل کنید که کتابخانه hook ساخته شده و قابل دسترس است:
```bash
# بررسی وجود کتابخانه
ls -la libforcebindip.so

# در صورت نیاز دوباره بسازید
make clean && make
```

### روش‌های عیب‌یابی

#### فعال کردن حالت پرجزئیات
```bash
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

#### بررسی تنظیمات شبکه
```bash
# لیست interface ها
ip addr show

# بررسی routing
ip route show

# تست اتصال
ping -I 192.168.1.3 google.com
```

#### تست دستی
```bash
# تست بدون ForceBindIP
./test_ipv4

# تست با ForceBindIP
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

### نمونه‌های خروجی

#### IPv4 Binding موفق
```
$ ./forcebindip -v -4 192.168.1.3 ./test_ipv4
Verbose mode enabled
Set preferred IP to IPv4: 192.168.1.3
Executing: ./test_ipv4 
Using hook library: ./libforcebindip.so
ForceBindIP Linux hooks initializing...
Preferred IPv4: 192.168.1.3
Preferred Interface: 
Bind TCP: yes
Bind UDP: yes
Linux hooks initialized successfully
Socket 3 created: domain=2, type=1, protocol=0
Bound to IPv4: 192.168.1.3:0
TCP socket 3 connected from: 192.168.1.3:42019
Connected from local IP: 192.168.1.3
```

#### خروجی لیست Interface
```
$ ./forcebindip -l
Available Network Interfaces:
========================================

Interface Name: wlo1
Interface ID: wlo1
Description: Linux Network Interface
Status: Up
IPv4 Address: 192.168.1.3

Interface Name: eth0
Interface ID: eth0
Description: Linux Network Interface
Status: Up
IPv4 Address: 10.0.0.5
```

### اطلاعات توسعه

#### ساخت از کد منبع
```bash
# پاک کردن و ساخت
make clean

# ساخت تمام اجزا
make

# ساخت target های خاص
make libforcebindip.so
make forcebindip
make test_ipv4
```

#### روش‌های تست
```bash
# تست IPv4 binding
./forcebindip -v -4 192.168.1.3 ./test_ipv4

# تست interface binding
./forcebindip -v -i wlo1 ./test_ipv4

# تست با برنامه‌های واقعی
./forcebindip -v -4 192.168.1.3 curl http://httpbin.org/ip
```

#### جزئیات ساختار کد
```
FBI/
├── build/                           # پوشه خروجی ساخت
├── common/                          # Header های مشترک
│   ├── platform.h                   # Platform abstraction
│   ├── network_manager.h            # مدیریت interface شبکه
│   └── network_manager.cpp
├── dll/                             # کد منبع DLL ویندوز
├── forcebindip                      # اجرایی اصلی Linux
├── injector/                        # کد منبع injector ویندوز
├── injector_crossplatform/          # Injector چند پلتفرمه
│   └── injector.cpp
├── libforcebindip_simple.c          # کد منبع کتابخانه hook Linux (C)
├── libforcebindip.so                # کتابخانه hook Linux (کامپایل شده)
├── Makefile                         # سیستم ساخت Linux
├── README_FA.md                     # مستندات فارسی
├── README.md                        # مستندات انگلیسی
├── test_crossplatform/              # برنامه‌های تست چند پلتفرمه
│   ├── test_ipv4.cpp
│   └── test_ipv6.cpp
├── test_ipv4                        # اجرایی تست IPv4
├── test_ipv6                        # اجرایی تست IPv6
├── testv4/                          # کد منبع تست IPv4 اصلی
│   └── testv4.cpp
└── testv6/                          # کد منبع تست IPv6 اصلی
    └── testv6.cpp
```

## مشارکت
1. پروژه رو Fork کنید
2. شاخه ویژگی ایجاد کنید: `git checkout -b feature/my-feature`
3. روی هر دو پلتفرم تست کنید
4. Pull request ارسال کنید

## مجوز
بر اساس مجوز پروژه اصلی ForceBindIP.

---

**آماده کنترل ترافیک شبکه‌تون هستید؟**  
با `./forcebindip -l` شروع کنید تا interface هاتون رو ببینید، بعد برنامه‌هاتون رو مجبور کنید دقیقاً از مسیر شبکه‌ای که می‌خواید استفاده کنن!


</div>