#!/usr/bin/env python
#

import sys, argparse, logging, os

get_input = input

new_config = {
    'i2c' : { 'active' : True }
}

board_config_path = "boards"
linker_scripts = {
    'samd09' : 'scripts/samd09d14a_flash.ld'
}
pin_activity_led = "PA27"
max_addr_pins = 5

class Tc:
    def __init__(self, index, wo):
        self.index = index
        self.wo = wo

class Sercom:
    def __init__(self, index, pad):
        self.index = index
        self.pad = pad

class Samd09Pin:
    def __init__(self, name, adc, sercom, sercomalt, tc):
        self.name = name
        self.adc = adc
        self.sercom = sercom
        self.sercomalt = sercomalt
        self.tc = tc
        self.mux = None

    def can_i2c(self, alt=False):
        if alt:
            sercom = self.sercomalt
        else:
            sercom = self.sercom

        if sercom is not None:
            if sercom.pad == 0:
                return "SDA"
            if sercom.pad == 1:
                return "SCL"
        return False

    def number(self):
        return int(self.name[2:])

used_sercoms = []
used_tc = []

pinouts = {
    'samd09' : [
        Samd09Pin("PA02", 0, None, None, None),
        Samd09Pin("PA03", 1, None, None, None),
        Samd09Pin("PA04", 2, Sercom(0, 2), Sercom(0, 0), Tc(1, 0)),
        Samd09Pin("PA05", 3, Sercom(0, 3), Sercom(0, 1), Tc(1, 1)),
        Samd09Pin("PA06", 4, Sercom(0, 0), Sercom(0, 2), Tc(2, 0)),
        Samd09Pin("PA07", 5, Sercom(0, 1), Sercom(0, 3), Tc(2, 0)),
        Samd09Pin("PA08", None, Sercom(1, 2), Sercom(0, 2), None),
        Samd09Pin("PA09", None, Sercom(1, 3), Sercom(0, 3), None),
        Samd09Pin("PA10", 8, Sercom(0, 2), None, Tc(2, 0)),
        Samd09Pin("PA11", 9, Sercom(0, 3), None, Tc(2, 1)),
        Samd09Pin("PA14", 6, Sercom(0, 0), None, Tc(1, 0)),
        Samd09Pin("PA15", 7, Sercom(0, 1), None, Tc(1, 1)),
        Samd09Pin("PA16", None, Sercom(1, 2), None, Tc(1, 0)),
        Samd09Pin("PA17", None, Sercom(1, 3), None, Tc(1, 1)),
        Samd09Pin("PA22", None, Sercom(1, 0), None, Tc(1, 0)),
        Samd09Pin("PA23", None, Sercom(1, 1), None, Tc(1, 1)),
        Samd09Pin("PA27", None, None, None, None),
        Samd09Pin("PA28", None, None, None, None),
        Samd09Pin("PA30", None, Sercom(1, 0), Sercom(1, 2), Tc(2, 0)),
        Samd09Pin("PA31", None, Sercom(1, 2), Sercom(1, 3), Tc(2, 1)),
        Samd09Pin("PA24", None, Sercom(1, 2), None, None),
        Samd09Pin("PA25", None, Sercom(1, 3), None, None),
    ],
    'samd21' : []
}

used_pins = {
    'samd09' : [
        pinouts['samd09'][18], pinouts['samd09'][19], pinouts['samd09'][17] #SWCLK, SDWIO, RESET
    ]
}

def pin_is_used(name):
    name = name.upper()
    for pin in used_pins[new_config['chipset']]:
        if pin.name == name:
            return True
    return False

def is_a_pin(name):
    name = name.upper()
    for pin in pinouts[new_config['chipset']]:
        if pin.name == name:
            return True
    return False

def get_pin_index(name):
    name = name.upper()
    for ix,pin in enumerate(pinouts[new_config['chipset']]):
        if pin.name == name:
            return ix
    return -1

def get_available_sda():
    pins = []
    for pin in pinouts[new_config['chipset']]:
        if pin not in used_pins[new_config['chipset']]:
            if pin.can_i2c() == "SDA" and pin.sercom is not None and pin.sercom.index not in used_sercoms:
                pins.append( (pin, pin.sercom, 2) )
            if pin.can_i2c(True) == "SDA" and pin.sercomalt is not None and pin.sercomalt.index not in used_sercoms:
                pins.append( (pin, pin.sercom, 3) )
    return pins

def get_available_scl():
    pins = []
    for pin in pinouts[new_config['chipset']]:
        if pin not in used_pins[new_config['chipset']]:
            if pin.can_i2c() == "SCL" and pin.sercom is not None and pin.sercom.index not in used_sercoms:
                pins.append( (pin, pin.sercom, 2) )
            if pin.can_i2c(True) == "SCL" and pin.sercomalt is not None and pin.sercomalt.index not in used_sercoms:
                pins.append( (pin, pin.sercom, 3) )
    return pins

def get_available_adc():
    pins = []
    for pin in pinouts[new_config['chipset']]:
        if pin not in used_pins[new_config['chipset']]:
            if pin.adc is not None:
                pins.append(pin)
    return pins

def get_available_pwm():
    pins = []
    for pin in pinouts[new_config['chipset']]:
        if pin not in used_pins[new_config['chipset']]:
            if pin.tc is not None:
                used = False
                for tc in used_tc:
                    if tc.index == pin.tc.index and tc.wo == pin.tc.wo: #we want independent channels
                        used = True
                        break
                if not used:
                    pins.append(pin)
    return pins

def get_available_gpio():
    pins = []
    for pin in pinouts[new_config['chipset']]:
        if pin not in used_pins[new_config['chipset']]:
            pins.append(pin)
    return pins

def generate_files():
    path = board_config_path + "/" + new_config['name']
    os.makedirs(path)
    f = open(path + "/board.mk", 'w')
    f.write("CHIP_FAMILY = %s\n" % new_config['chipset'].upper())
    f.write("CHIP_VARIANT = SAMD09D14A\n") #TODO:
    f.write("BOARD_NAME = %s\n" % new_config['name'].upper())
    f.write("LINKER_SCRIPT = %s\n" % linker_scripts[new_config['chipset']])
    f.close()

    f = open(path + "/board_config.h", 'w')

    f.writelines(["#ifndef SEESAW_DEVICE_CONFIG_H\n",
                "#define SEESAW_DEVICE_CONFIG_H\n"])

    f.write("\n#define PRODUCT_CODE %i\n" % new_config['pid'])

    if new_config['i2c']['no_led']:
        f.write("#define CONFIG_NO_ACTIVITY_LED\n")

    f.write("#define CONFIG_NO_EEPROM\n")

    f.writelines(["\n","//* ============== POOL SIZES =================== *//\n",
    "#define	EVT_SIZE_SMALL 16\n",
    "#define EVT_SIZE_MEDIUM 32\n",
    "#define	EVT_SIZE_LARGE 64\n",
    "#define	EVT_COUNT_SMALL 16\n",
    "#define	EVT_COUNT_MEDIUM 8\n",
    "#define	EVT_COUNT_LARGE 1\n",
    "\n",
    "//* ============== ADC =================== *//\n"])

    f.write("#define CONFIG_ADC %i\n" % (0 if not new_config['adc'] else 1))
    if new_config['chipset'] == 'samd09':
      max_adc = 4
    elif new_config['chipset'] == 'samd21':
      max_adc = 8
    for ix in range(max_adc):
        f.write("#define CONFIG_ADC_INPUT_%i " % ix)
        if ix > len(new_config['adc'])-1:
            f.write("0\n")
        else:
            f.write("1\n#define CONFIG_ADC_INPUT_%i_PIN %i\n\n" % (ix, new_config['adc'][ix].number()))

    f.writelines(["//* ============== DAC =================== *//\n",
    "#define CONFIG_DAC 0\n\n",
    "//* ============== TIMER =================== *//\n"])

    f.write("#define CONFIG_TIMER %i\n" % (0 if not new_config['pwm'] else 1))
    if new_config['chipset'] == 'samd09':
      max_pwm = 4
    elif new_config['chipset'] == 'samd21':
      max_pwm = 12
    for ix in range(max_pwm):
        f.write("#define CONFIG_TIMER_PWM_OUT%i " % ix)
        if ix > len(new_config['pwm'])-1:
            f.write("0\n")
        else:
            f.write("1\n#define CONFIG_TIMER_PWM_OUT%i_PIN %i\n" % (ix, new_config['pwm'][ix].number()))
            f.write("#define CONFIG_TIMER_PWM_OUT%i_TC TC%i\n" % (ix, new_config['pwm'][ix].tc.index))
            f.write("#define CONFIG_TIMER_PWM_OUT%i_WO %i\n" % (ix, new_config['pwm'][ix].tc.wo))

    f.write("\n//* ============== INTERRUPT =================== *//\n")

    f.write("#define CONFIG_INTERRUPT %i\n" % (0 if new_config['interrupt'] is None else 1))
    f.write("#define CONFIG_INTERRUPT_PIN %i\n" % (0 if new_config['interrupt'] is None else new_config['interrupt'].number()))

    f.write("\n//* ============== I2C SLAVE =================== *//\n")

    f.write("#define CONFIG_I2C_SLAVE %i\n" % (0 if new_config['i2c']['active'] is False else 1))
    if new_config['i2c']['active']:
        f.write("#define CONFIG_I2C_SLAVE_SERCOM SERCOM%i\n" % new_config['i2c']['sercom'])
        f.write("#define CONFIG_I2C_SLAVE_HANDLER SERCOM%i_Handler\n" % new_config['i2c']['sercom'])
        f.write("#define CONFIG_I2C_SLAVE_IRQn SERCOM%i_IRQn\n" % new_config['i2c']['sercom'])
        
        f.write("#define CONFIG_I2C_SLAVE_PIN_SDA %i\n" % new_config['i2c']['sda'].number())
        f.write("#define CONFIG_I2C_SLAVE_PIN_SCL %i\n" % new_config['i2c']['scl'].number())

        f.write("#define CONFIG_I2C_SLAVE_MUX %i\n" % new_config['i2c']['sda'].mux)

        f.write("#define CONFIG_I2C_SLAVE_FLOW_CONTROL %i\n" % (0 if new_config['i2c']['flow'] is None else 1))
        f.write("#define CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN %i\n" % (0 if new_config['i2c']['flow'] is None else new_config['i2c']['flow'].number()))
        
        f.write("#define CONFIG_I2C_SLAVE_ADDR 0x%s\n" % format(new_config['i2c']['address'], '02X'))

        if len(new_config['i2c']['addr_select']) == 0:
            f.write("#define CONFIG_NO_ADDR\n")
        else:
            for ix, p in enumerate(new_config['i2c']['addr_select']):
                if ix > 1:
                    f.write("#define CONFIG_ADDR_%i 1\n" % ix)
                f.write("#define PIN_ADDR_%i " % ix)
                f.write("%i\n" % p.number())

    f.writelines(["\n//* ============== SERCOM =================== *//\n",
    "#define CONFIG_SERCOM0 0\n",
    "#define CONFIG_SERCOM1 0\n",
    "#define CONFIG_SERCOM1 0\n",
    "#define CONFIG_SERCOM2 0\n\n",
    "//These are only available on samd21\n"
    "#define CONFIG_SERCOM3 0\n",
    "#define CONFIG_SERCOM4 0\n",
    "#define CONFIG_SERCOM5 0\n\n",
    "//* ============== DAP =================== *//\n",
    "#define CONFIG_DAP 0\n\n"
    "//* =========== NEOPIXEL ================ *//\n"])

    f.write("#define CONFIG_NEOPIXEL %i\n" % (0 if new_config['neopixel'] is None else 1))
    f.write(" #define CONFIG_NEOPIXEL_BUF_MAX %i\n" % (0 if new_config['neopixel'] is None else new_config['neopixel']))

    f.write("#endif\n")

    f.close()

def generate_preview():
    print("====== CONFIGURATION SUMMARY =====\n")
    print("NAME: %s" % new_config['name'])
    print("PID: %i" % new_config['pid'])
    print("CHIPSET: %s" % new_config['chipset'])
    if new_config['i2c']['active']:
        print("\nI2C:")
        print("\tADDRESS: 0x%s" % format(new_config['i2c']['address'], '02X'))
        print("\tSERCOM: SERCOM%i" % new_config['i2c']['sercom'])
        print("\tSDA: %s" % new_config['i2c']['sda'].name)
        print("\tSCL: %s" % new_config['i2c']['scl'].name)
        print("\tFLOW CONTROL: %s" % ("None" if new_config['i2c']['flow'] is None else new_config['i2c']['flow'].name))
        print("\tACTIVITY LED: %s" % (not new_config['i2c']['no_led']))
        if len(new_config['i2c']['addr_select']):
            print("\nADDR SELECT PINS: %i" % len(new_config['i2c']['addr_select']))
            for ix, p in enumerate(new_config['i2c']['addr_select']):
                print("\t%s" % p.name)
        else:
            print("\tADDR SELECT PINS: None\n")

    if new_config['adc']:
        print("\nADC CHANNELS: %i" % len(new_config['adc']))
        for ix, adc in enumerate(new_config['adc']):
            print("\t%s" % adc.name)

    if new_config['pwm']:
        print("\nPWM CHANNELS: %i" % len(new_config['pwm']))
        for ix, pwm in enumerate(new_config['pwm']):
            print("\t%s" % pwm.name)

    print("\nIRQ: %s" % ("None" if new_config['interrupt'] is None else new_config['interrupt'].name))
    print("NEOPIXEL BUFFER: %s" % new_config['neopixel'])
    print("\nAVAILABLE GPIOS:")
    for gpio in get_available_gpio():
        print("\t%s" % gpio.name)
    print("\n")

# Gather our code in a main() function
def main(args, loglevel):
  logging.basicConfig(format="%(levelname)s: %(message)s", level=loglevel)
  
  #logging.info("You passed an argument.")

  #NAME
  user_input = get_input("enter a name for your configuration: ")
  logging.debug("received name: %s" % user_input)

  if os.path.isdir(board_config_path + "/" + user_input):
      logging.error("directory "+board_config_path+"/%s already exists" % user_input)
      return
  else:
      new_config['name'] = user_input
      logging.debug("directory "+board_config_path+"/%s does not yet exist" % user_input)

  #CHIPSET
  user_input = get_input("select a chipset:\n [0] samd09 \n [1] samd21 \n: ")
  if user_input == "0":
      new_config['chipset'] = "samd09"
  elif user_input == "1":
      new_config['chipset'] = "samd21"
      #TODO: fix
      logging.error("SAMD21 support coming soon")
      return
  else:
      logging.error("unrecognized option")
      return

  logging.debug("chipset %s selected" % new_config['chipset'])

  #BOOTLOADER
  if new_config['chipset'] == "samd21":
      user_input = get_input("will you use a bootloader? [y/n]: ")
      if user_input == "y":
          new_config['bootloader'] = True
      elif user_input == "n":
          new_config['bootloader'] = False
      else:
          logging.error("unrecognized option")
          return

      logging.debug("selected uses bootloader: %s" % new_config['bootloader'])

  #PID
  user_input = get_input("what's the product id? : ")
  if not user_input.isdigit():
      logging.error("the product ID must be a positive integer")
      return
  new_config['pid'] = int(user_input)
  logging.debug("selected pid: %i" % new_config['pid'])

  #I2C
  user_input = get_input("will you use the i2c slave interface? [y/n]: ")
  if user_input == "y":
    new_config['i2c'] = {'active' : True}
  elif user_input == "n":
    new_config['i2c'] = {'active' : False}
  else:
    logging.error("unrecognized option")
    return

  if new_config['i2c']['active']:
    #I2C SERCOM
    sdapins = get_available_sda()
    istring = "which pin for SDA?\n"
    for ix,cfg in enumerate(sdapins):
        istring = istring + " [" + str(ix) + "] " + cfg[0].name + " (sercom" + str(cfg[1].index) + ")\n"

    istring = istring + ": "
    user_input = get_input(istring)
    
    if user_input.isdigit() and int(user_input) < len(sdapins):
        cfg = sdapins[int(user_input)]
        new_config['i2c']['sercom'] = cfg[1].index
        new_config['i2c']['sda'] = cfg[0]
        new_config['i2c']['mux'] = cfg[2]
        cfg[0].mux = cfg[2]
        used_pins[new_config['chipset']].append(cfg[0])
        logging.debug("selected SDA pin: %s" % new_config['i2c'])
        logging.debug("selected I2C Sercom: %s" % new_config['i2c']['sercom'])
    else:
        logging.error("unrecognized option")
        return

    sclpins = get_available_scl()
    accepatable_sclpins = []
    istring = "which pin for SCL?\n"
    for cfg in sclpins:
        if cfg[1].index == new_config['i2c']['sercom'] and cfg[2] == new_config['i2c']['mux']:
            accepatable_sclpins.append(cfg)
    sclpins = accepatable_sclpins
    for ix,cfg in enumerate(sclpins):
        istring = istring + " [" + str(ix) + "] " + cfg[0].name + " (sercom" + str(cfg[1].index) + ")\n"

    istring = istring + ": "
    user_input = get_input(istring)
    
    if user_input.isdigit() and int(user_input) < len(sclpins):
        cfg = sclpins[int(user_input)]
        new_config['i2c']['sercom'] = cfg[1].index
        new_config['i2c']['scl'] = cfg[0]
        cfg[0].mux = cfg[2]
        used_pins[new_config['chipset']].append(cfg[0])
        logging.debug("selected SCL pin: %s" % new_config['i2c']['scl'].name)
    else:
        logging.error("unrecognized option")
        return

    #mark sercom is now used
    used_sercoms.append(new_config['i2c']['sercom'])

    #ADDRESS
    user_input = get_input("which I2C address? [default 0x49]: ")
    if user_input == "":
        new_config['i2c']['address'] = 0x49
    else:
        new_config['i2c']['address'] = int(user_input, 0)

    logging.debug("selected i2c address: 0x%s"  % format(new_config['i2c']['address'], '02X'))
    if new_config['i2c']['address'] > 0x7F:
        logging.error("the I2C address cannot be greater than 7 bits in length.")
        return

    #ADDRESS SELECT
    new_config['i2c']['addr_select'] = []
    while(len(new_config['i2c']['addr_select']) < max_addr_pins):
        user_input = get_input("add a pin for address select? (ex. enter name as 'PA10' or leave blank for no more address select) :")
        if user_input == "":
            break
        else:
            if get_pin_index(user_input) > -1:
                if pin_is_used(user_input):
                    logging.error("pin is used!")
                    return
                else:
                    selected_pin = pinouts[new_config['chipset']][get_pin_index(user_input)]
                    new_config['i2c']['addr_select'].append(selected_pin)
                    used_pins[new_config['chipset']].append(selected_pin)
                    logging.debug("selected addr pin: %s" % selected_pin.name)
            else:
                logging.error("unrecognized option")
                return

    #ACTIVITY LED
    user_input = get_input("will you use the activity LED? [y/n]: ")
    if user_input == "y":
      new_config['i2c']['no_led'] = False
      used_pins[new_config['chipset']].append(pinouts[new_config['chipset']][get_pin_index(pin_activity_led)])
    elif user_input == "n":
      new_config['i2c']['no_led'] = True
    else:
      logging.error("unrecognized option")
      return

    #FLOW
    user_input = get_input("which pin for flow control? (ex. enter name as 'PA10' or leave blank for no flow control) :")
    if user_input == "":
        new_config['i2c']['flow'] = None
    else:
        if get_pin_index(user_input) > -1:
            if pin_is_used(user_input):
                logging.error("pin is used!")
                return
            else:
                selected_pin = pinouts[new_config['chipset']][get_pin_index(user_input)]
                new_config['i2c']['flow'] = selected_pin
                used_pins[new_config['chipset']].append(selected_pin)
                logging.debug("selected flow pin: %s" % new_config['i2c']['flow'].name)
        else:
            logging.error("unrecognized option")
            return
        
  #ADC
  new_config['adc'] = []
  if new_config['chipset'] == 'samd09':
      max_adc = 4
  elif new_config['chipset'] == 'samd21':
      max_adc = 8

  ix = 0
  while(ix < max_adc):
    available_adcs = get_available_adc()
    istring = "select an ADC pin or leave blank for no more ADC\n"
    for jx, pin in enumerate(available_adcs):
        istring = istring + " [" + str(jx) + "] " + pin.name + "\n"
    istring = istring + ": "
    user_input = get_input(istring)
    if user_input == "":
        break
    if user_input.isdigit() and int(user_input) < len(available_adcs):
        new_config['adc'].append(available_adcs[int(user_input)])
        used_pins[new_config['chipset']].append(available_adcs[int(user_input)])
    else:
        logging.error("unrecognized option")
        return
    ix = ix + 1

  #PWM
  new_config['pwm'] = []
  if new_config['chipset'] == 'samd09':
      max_pwm = 4
  elif new_config['chipset'] == 'samd21':
      max_pwm = 12

  ix = 0
  while(ix < max_pwm):
    available_pwm = get_available_pwm()
    istring = "select a PWM pin or leave blank for no more PWM\n"
    for jx, pin in enumerate(available_pwm):
        istring = istring + " [" + str(jx) + "] " + pin.name + "\n"
    istring = istring + ": "
    user_input = get_input(istring)
    if user_input == "":
        break
    if user_input.isdigit() and int(user_input) < len(available_pwm):
        new_config['pwm'].append(available_pwm[int(user_input)])
        used_pins[new_config['chipset']].append(available_pwm[int(user_input)])
        used_tc.append(available_pwm[int(user_input)].tc)
    else:
        logging.error("unrecognized option")
        return
    ix = ix + 1

  #INTERRUPT
  available_int = get_available_gpio()
  istring = "select an IRQ pin or leave blank for none\n"
  for jx, pin in enumerate(available_int):
    istring = istring + " [" + str(jx) + "] " + pin.name + "\n"
  istring = istring + ": "
  user_input = get_input(istring)
  if user_input == "":
    new_config['interrupt'] = None
  elif user_input.isdigit() and int(user_input) < len(available_int):
    new_config['interrupt'] = available_int[int(user_input)]
    used_pins[new_config['chipset']].append(available_int[int(user_input)])
  else:
    logging.error("unrecognized option")
    return

  #NEOPIXEL
  user_input = get_input("enter a NeoPixel buffer size in bytes or leave blank for none : ")
  if user_input == "":
      new_config['neopixel'] = None
  elif not user_input.isdigit():
      logging.error("bufsize must be a positive integer")
      return
  else:
    new_config['neopixel'] = int(user_input)

  generate_preview()

  user_input = get_input("press enter to generate or Ctrl+C to cancel")

  generate_files()

  print("configuration generated. Please run 'make BOARD=%s'\n" % new_config['name'])
  
 
# Standard boilerplate to call the main() function to begin
# the program.
if __name__ == '__main__':
  parser = argparse.ArgumentParser( 
                                    description = "Generates a new seesaw configuration",
                                    epilog = "As an alternative to the commandline, params can be placed in a file, one per line, and specified on the commandline like '%(prog)s @params.conf'.",
                                    fromfile_prefix_chars = '@' )
  # TODO Specify your real parameters here.
  parser.add_argument(
                      "-v",
                      "--verbose",
                      help="increase output verbosity",
                      action="store_true")
  args = parser.parse_args()
  
  # Setup logging
  if args.verbose:
    loglevel = logging.DEBUG
  else:
    loglevel = logging.INFO
  
  main(args, loglevel)
