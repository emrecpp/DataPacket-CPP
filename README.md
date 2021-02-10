# DataPacket for C++
Store data as packet. Send, Recv, Encrypt it.

For C#: https://github.com/emrecpp/DataPacket-CSharp

For Python: https://github.com/emrecpp/PacketHandler

# Example Usage
```cpp

#include <iostream>
#include <Windows.h>
#include "Packet.h"


int main(){
    Packet paket = Packet(0xDFFF, false);
    SYSTEMTIME NowTime;
    GetSystemTime(&NowTime);
    
    paket << "Emre" << 123 << 1.5f << NowTime;
    
    string name;
    int number;
    float f;
    SYSTEMTIME SystemTimeStruct;
    paket >> name >> number >> f >> SystemTimeStruct;
    printf("Opcode: %d\n\nName: %s\nNumber: %d\nFloat: %f\nSystemTime: %d-%02d-%02d\n", paket.GetOpcode(), name.c_str(), number, f,
      SystemTimeStruct.wYear, SystemTimeStruct.wMonth, SystemTimeStruct.wDay);
      
    paket.Print();
    std::getchar();
    return 0;
}
```


#### Output:
```
Opcode: 57343 (0xDFFF)
Name: Emre
Number: 123
Float: 1.500000
SystemTime: 2021-02-09


Little Endian

Normal Print:

00000000 df ff 00 03 00 00 00 00 00 0d 45 6d 72 65 20 44    ..........Emre.D.........<.....................
00000016 65 6d 69 72 63 61 6e 7b 00 00 00 00 00 c0 3f e5    emircan{......?.........5="....................
00000032 07 02 00 02 00 09 00 14 00 0d 00 25 00 23 01       ...........%.#...........<.......f..P*.........

Big Endian

Normal Print:

00000000 df ff 00 03 00 00 00 00 00 0d 45 6d 72 65 20 44    ..........Emre.D........E7.....................
00000016 65 6d 69 72 63 61 6e 00 00 00 7b 3f c0 00 00 e5    emircan...{?............}7.....................
00000032 07 02 00 02 00 09 00 14 00 0e 00 0f 00 cd 00       ........................u7.....................
```

