SET fh="C:\Documents and Settings\user\��� ���㬥���\Arduino\sketch_feb11b\prog.h"
SET fhex=temp.hex

hex2bin %fhex%

echo // %date% %time%                   >%fh%
echo const uint8_t my_prog[] PROGMEM={ >>%fh%
bin2h.exe %fhex:~0,-3%bin              >>%fh%
echo };                                >>%fh%
