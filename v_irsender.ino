/*
 * v_irsender: Sending IR codes, which comes from Serial port.
 * An IR LED must be connected to Arduino PWM pin 3.
 * Serial port command format:
 * <IR protocol> <IR data>
 * for example:
 * PRONTO 0000 006d 0026 0000 0155 00aa 0016 0015 0016 0015 0016 0040 0016 0015 0016 0015 0016 0014 0016 0014 0016 0015 0016 0040 0016 0040 0016 0015 0016 0040 0016 0040 0016 0040 0016 0040 0016 0040 0016 0040 0016 0014 0016 0040 0016 0015 0016 0015 0016 0014 0016 0040 0016 0040 0016 0014 0016 0040 0016 0015 0016 0040 0016 0040 0016 0040 0016 0014 0016 0015 0016 060b 0155 0055 0016 0e58 0155 0055 0016 00aa
 * LG 4 -1 197
 * SONY-RAW A90 12
 *
 * v1.0 (2016-01-06)
 * Copyright 2016 Valerii Vyshniak
 * http://www.vi-soft.com.ua
 */

#include <IRremote.h>

IRsend irsend;

bool parse_data_nbits(String& str, unsigned long& data, int& nbits, bool& irresult)
{
  data = 0;
  nbits = 0;
  int parsed = 0;
  int params = sscanf(str.c_str(), "%lx %d%n", &data, &nbits, &parsed);
  if (params == 2 && parsed == str.length() && nbits >= 0 && nbits <= 32)
  {
    Serial.print(data, HEX);
    Serial.print(F(" "));
    Serial.println(nbits, DEC);
    irresult = true;
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Protocol parameters are wrong. Supported format: <HEX data> <DEC nbits>"));
    irresult = false;
    return false;
  }
}

bool parse_data_nbits_repeat(String& str, unsigned long& data, int& nbits, bool& repeat, bool& irresult)
{
  data = 0;
  nbits = 0;
  repeat = false;
  int repeat_int = 0;
  int parsed = 0;
  int params = sscanf(str.c_str(), "%lx %d %d%n", &data, &nbits, &repeat, &parsed);
  if (params == 3 && parsed == str.length() && nbits >= 0 && nbits <= 32
       && repeat_int >= 0 && repeat_int <= 1)
  {
    Serial.print(data, HEX);
    Serial.print(F(" "));
    Serial.print(nbits, DEC);
    Serial.print(F(" "));
    Serial.println(repeat_int, DEC);
    repeat = repeat_int;
    irresult = true;
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Protocol parameters are wrong. Supported format: <HEX data> <DEC nbits> <DEC repeat>"));
    irresult = false;
    return false;
  }
}

bool parse_device_subdevice_function(String& str, int& device, int& subdevice, int& function, bool& irresult)
{
  device = -1;
  subdevice = -1;
  function = -1;
  int parsed = 0;
  int params = sscanf(str.c_str(), "%d %d %d%n", &device, &subdevice, &function, &parsed);
  if (params == 2)
  {
    params = sscanf(str.c_str(), "%d %d%n", &device, &function, &parsed);
    subdevice = -1;
    params = 3;
  }
  if (params == 3 && parsed == str.length() && device >= 0 && device <= 255
      && function >= 0 && function <= 255 && subdevice >= -1 && subdevice <= 255)
  {
    Serial.print(device, DEC);
    Serial.print(F(" "));
    Serial.print(subdevice, DEC);
    Serial.print(F(" "));
    Serial.println(function, DEC);
    irresult = true;
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Protocol parameters are wrong. Supported format: <DEC device> [DEC subdevice] <DEC function>"));
    irresult = false;
    return false;
  }
}

bool parse_code(String& str, int& code, bool& irresult)
{
  code = -1;
  int parsed = 0;
  int params = sscanf(str.c_str(), "%d%n", &code, &parsed);
  if (params == 1 && parsed == str.length() && code >= 0)
  {
    Serial.println(code, DEC);
    irresult = true;
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Protocol parameters are wrong. Supported format: <DEC code>"));
    irresult = false;
    return false;
  }
}

bool parse_address_command(String& str, unsigned int& address, unsigned int& command, bool& irresult)
{
  address = -1;
  command = -1;
  int parsed = 0;
  int params = sscanf(str.c_str(), "%d %d%n", &address, &command, &parsed);
  if (params == 2 && parsed == str.length() && (int)address >= 0 && (int)command >= 0)
  {
    Serial.print(address, DEC);
    Serial.print(F(" "));
    Serial.println(command, DEC);
    irresult = true;
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Protocol parameters are wrong. Supported format: <DEC address> <DEC command>"));
    irresult = false;
    return false;
  }
}

unsigned long revbits(unsigned long in)
{
  unsigned long result = 0;
  for (int i = 0; i < sizeof(unsigned long) * 8; i++)
  {
      result <<= 1;
      result |= in & 1;
      in >>= 1;
  }
  return result;
}

void setup()
{
  // start serial port at 115200 bps and wait for port to open:
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("v_irsender v1.0: Sending CIR codes, which comes from Serial port."));
  Serial.println(F("Serial port command format:"));
  Serial.println(F("<IR protocol> <IR data> <new_line>"));
  Serial.println(F("for example:"));
  Serial.println(F("PRONTO 0000 006d 0026 0000 0155 00aa 0016 0015 0016 0015 0016 0040 0016 0015 0016 0015 0016 0014 0016 0014 0016 0015 0016 0040 0016 0040 0016 0015 0016 0040 0016 0040 0016 0040 0016 0040 0016 0040 0016 0040 0016 0014 0016 0040 0016 0015 0016 0015 0016 0014 0016 0040 0016 0040 0016 0014 0016 0040 0016 0015 0016 0040 0016 0040 0016 0040 0016 0014 0016 0015 0016 060b 0155 0055 0016 0e58 0155 0055 0016 00aa"));
}

void loop()
{
  if (Serial.available() > 0)
  {
    // read the data:
    String cmd_str = Serial.readStringUntil('\n');

    //remove invalid symbols before command
    int i;
    for (i = 0; i < cmd_str.length(); i++)
    {
      if (cmd_str[i] >= 'A' && cmd_str[i] <= 'Z')
      {
        break;
      }
    }
    cmd_str.remove(0, i);
    cmd_str.trim();
    if (cmd_str.length() != 0)
    {
      i = cmd_str.indexOf(' ');
      if (i == -1)
      {
        i = cmd_str.length();
      }
      // protocol type:
      String irprot_str = cmd_str.substring(0, i);
      // command data:
      String irdata_str = cmd_str.substring(i, cmd_str.length());

      unsigned long data = 0;
      int nbits = 0;
      int device = -1;
      int subdevice = -1;
      int function = -1;
      bool irresult = true;

      irprot_str.trim();
      irdata_str.trim();
      Serial.print(F("Req: "));
      Serial.print(irprot_str);
      Serial.print(F(" "));

      if (irprot_str == "PRONTO")
      {
        Serial.println(irdata_str);
        irresult = irsend.sendPronto(irdata_str.c_str(), PRONTO_ONCE, PRONTO_FALLBACK);
      }

      else if (irprot_str == "PRONTO-REPEAT")
      {
        Serial.println(irdata_str);
        irresult = irsend.sendPronto(irdata_str.c_str(), PRONTO_REPEAT, PRONTO_FALLBACK);
      }

      else if (irprot_str == "LG-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendLG(data, nbits);
        }
      }

      else if (irprot_str == "LG")
      {
        if (parse_device_subdevice_function(irdata_str, device, subdevice, function, irresult))
        {
          if (subdevice < 0)
          {
            subdevice = ~device;
          }
          data = 0;
          data |= ((unsigned long)(device & 0xFF)) << 0;
          data |= ((unsigned long)(subdevice & 0xFF)) << 8;
          data |= ((unsigned long)(function & 0xFF)) << 16;
          data |= ((unsigned long)(~function & 0xFF)) << 24;
          irsend.sendLG(revbits(data), 32);
        }
      }

      else if (irprot_str == "RC5-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendRC5(data, nbits);
        }
      }

      else if (irprot_str == "RC6-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendRC6(data, nbits);
        }
      }

      else if (irprot_str == "NEC-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendNEC(data, nbits);
        }
      }

      else if (irprot_str == "NEC")
      {
        if (parse_device_subdevice_function(irdata_str, device, subdevice, function, irresult))
        {
          if (subdevice < 0)
          {
            subdevice = ~device;
          }
          data = 0;
          data |= ((unsigned long)(device & 0xFF)) << 0;
          data |= ((unsigned long)(subdevice & 0xFF)) << 8;
          data |= ((unsigned long)(function & 0xFF)) << 16;
          data |= ((unsigned long)(~function & 0xFF)) << 24;
          irsend.sendNEC(revbits(data), 32);
        }
      }

      else if (irprot_str == "SONY-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendSony(data, nbits);
        }
      }

      else if (irprot_str == "PANASONIC-RAW")
      {
        unsigned long data = 0;
        int nbits = 0;
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendPanasonic(data, nbits);
        }
      }

      else if (irprot_str == "JVC-RAW")
      {
        bool repeat = false;
        if (parse_data_nbits_repeat(irdata_str, data, nbits, repeat, irresult))
        {
          irsend.sendJVC(data, nbits, repeat);
        }
      }

      else if (irprot_str == "SAMSUNG-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendSAMSUNG(data, nbits);
        }
      }

      else if (irprot_str == "WHYNTER-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendWhynter(data, nbits);
        }
      }

      else if (irprot_str == "AIWARCT501")
      {
        int code = 0;
        if (parse_code(irdata_str, code, irresult))
        {
          irsend.sendAiwaRCT501(code);
        }
      }

      else if (irprot_str == "DISH-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendDISH(data, nbits);
        }
      }

      else if (irprot_str == "SHARP-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendSharpRaw(data, nbits);
        }
      }

      else if (irprot_str == "SHARP")
      {
        unsigned int address = 0;
        unsigned int command = 0;
        if (parse_address_command(irdata_str, address, command, irresult))
        {
          irsend.sendSharp(address, command);
        }
      }

      else if (irprot_str == "DENON-RAW")
      {
        if (parse_data_nbits(irdata_str, data, nbits, irresult))
        {
          irsend.sendDenon(data, nbits);
        }
      }

      else
      {
        Serial.println(F("ERROR: Unknown IR protocol requested"));
        irresult = false;
      }

      if (irresult)
      {
        Serial.println(F("Res: OK"));
      }
      else
      {
        Serial.println(F("Res: ERROR"));
      }
    }
  }
}
