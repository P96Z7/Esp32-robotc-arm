# Hardware 

## Mapping
Conections:

| Atuator | Potentiometer | Servo |
| --- | --- | --- |
| Shoulder | GPIO 32 | GPIO 18 |
| Elbow | GPIO 33 | GPIO 19 |
| Hand | GPIO 34 | GPIO 21 |
| Claw | GPIO 35 | GPIO 22 |

## Alimentation and Notes

- Ligue os terminais externos de cada potenciômetro de 10 kΩ a 3,3 V e GND; o cursor que vai ao GPIO indicado passa por um filtro de passa baixa.
- The Low-Band Filter use one 100uF ceramic Capacitor and one 1kΩ Resistor.
- CAUTION:dont apply 5 volts in ADC inputs
- Servos being alimented for the 5v font and more or less 2A .
- Unify all GNDS in the same local, in my example I used the GND of ESP32.
- Check the Servos polarity and if is possible, dont conect without confirm the polarity of wires 
- Dont forget the electrolitic 1000uF capacitor to filter channel the noisy
- VCC and GND of servo is paralel conectec with a 100uF ceramic capacitor to reduce the Voltage peak
- To guarantee the servos will not die with some peak of current, it's a good practice put one 3A fuse :). 


## How it works?

<p align="center">
  <img
    src="../media/schematiks.png"
    alt="Hardware do braço robótico"
    width="600"
  >
</p>

    
    

