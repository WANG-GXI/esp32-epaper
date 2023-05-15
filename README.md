# esp32-epaper
# 使用VScode-platformio
## 注意事项：
          【1】建议修改分区。我的方案如下
          # Name,   Type, SubType, Offset,  Size, Flags
          nvs,      data, nvs,     0x9000,  0x5000,
          otadata,  data, ota,     0xe000,  0x2000,
          app0,     app,  ota_0,   0x10000, 0x300000,
          spiffs,   data, spiffs,  0x310000,0xF0000,
          【2】在GxEPD2_display_selection_new_style.h中修改引脚，直接找ESP32就行了，用自己的引脚
          【3】如果下载不进去可以把墨水屏断电，一般不会出现这个问题
          【4】如果报错找不到SPI.h的话亲测在ini文件中加入下面这行就可以了
          lib_ldf_mode = deep+
          【5】VScode虽然编译速度快，同时还可以支持转到定义，但是有些操作不如Arduino人性化，下载第三方库后不知道如何操作的小白得花点时间研究
          【6】我的代码中默认显示城市为天津，可以自己更换
