# Arduino4x3keypad

- swappable Arduino nano
- ws2812b addressable per-key RGB lighting
- hot-swappable switches
- effects showcase: https://youtu.be/Na9jvEjyg58
- current effects: Rainbow,
                   StaticRainbow,
                   Splash,
                   StaticLight,
                   Breathing,
                   FractionalDrawingTest2d,
                   SpinningRainbow,
                   Ripple,
                   AntiRipple,
                   Stars,
                   Raindrop,
                   Snake,
                   ShootingParticles,
                   Fire,
                   WhacAMole (game),
                   TicTacToe (game),
                   BullsNCows (game)

- ! because of the pin headers for the external swappable arduino board, the total height will be much taller than a solder-on one
- ! due to usable space being too small, the 5v pin needs to be manually soldered on using extra wires

- ! because of the usage of arduino nano (can't be connected as a USB HID hardware), an external software is required to receive the keypad messages and tell the computer keypresses, the driver and GUI written in Autoit is on github (https://github.com/Alan72104/KeypadDriver)

acrylic case schematic is also available (https://oshwlab.com/alan72104/4x3-keypad)
