<p align="center">
  <img width="750" alt="Keypad logo" src="that.png">
</p>

# Arduino4x3keypad

- [KeypadDriver](https://github.com/Alan72104/KeypadDriver)
- effect showcase: https://youtu.be/Na9jvEjyg58
- acrylic case schematic: https://oshwlab.com/alan72104/4x3-keypad

This was a fun project

### Features

- swappable Arduino nano
- ws2812b addressable per-key RGB lighting
- hot-swappable switches
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

### To improve

- If using pin headers for the arduino board to be swappable, the total height will be much taller than a solder-on one

- Due to the usable space being too small, the 5v pin needs to be manually soldered on using extra wires lol

- Because of the limitation of arduino nano (can't be connected as a USB HID hardware), an external software is required to translate the keypad serial messages to key presses ([KeypadDriver](https://github.com/Alan72104/KeypadDriver))
