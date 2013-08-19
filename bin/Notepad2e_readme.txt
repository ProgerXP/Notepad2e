Настройки INI файла . 
Касаются только НОВОЙ функцональности Notepad2e (НЕ Notepad2)!
Должны лежать строго в секции [e-settings]
######################################
* debug_log - включить/выключить лог(1 или 0)
######################################
* css_settings - тип CSS , по которому будет работать подсветка документа. Сумма битов
1 - sassy, 
2 - less , 
4 - hss
#######################################
* shell_menu_type - тип контекного shell меню.Сумма битов.

- NORMAL              0x00000000
- DEFAULTONLY         0x00000001
- VERBSONLY           0x00000002
- EXPLORE             0x00000004
- NOVERBS             0x00000008
- CANRENAME           0x00000010
- NODEFAULT           0x00000020
- INCLUDESTATIC       0x00000040
- ITEMMENU            0x00000080
- EXTENDEDVERBS       0x00000100
- DISABLEDVERBS       0x00000200
- ASYNCVERBSTATE      0x00000400
- OPTIMIZEFORINVOKE   0x00000800
- SYNCCASCADEMENU     0x00001000
- DONOTPICKDEFAULT    0x00002000
- RESERVED            0xffff0000

######################################
* use_prefix_in_open_dialog - испоьзовать ли открытие файла по префиксу в диалоге. 0 или 1

######################################