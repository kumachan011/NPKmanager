A simple plug and play program that allows you to UNPACK and REPACK Nitroplus .NPK archives easily, as well as find the decryption keys of those games.

The program allows you to insert your own decryption key and NPK version, but there are a few officially supported games.
Currently supported games:
  - You and Me and Her: A Love Story
  - Tokyo Necro
  - Song of Saya

For yet unsupported games, I have made a simple Nitroplus decryption key finder, which can be launched with the KeyFinderInjector.exe program. In case of any errors encountered, the Nitroplus game might not be valid, or it may be a fault in my code.

  <img width="410" height="213" alt="image" src="https://github.com/user-attachments/assets/7dc66662-e8a6-4eee-b85b-11e0ae55b2d0" />



How to unpack .NPK files:
    - It's as easy as just drag and dropping the desired NPK archive into the command prompt. Usually, the most commonly wanted files are things such as cg.npk, script.npk and sound.npk, but prodding around in the other files can be entertaining.

  <img width="425" height="296" alt="image" src="https://github.com/user-attachments/assets/9a46241b-0b2c-4b5f-a265-a279d46ddd5d" />


How to repack .NPK files:
    - You can drag and drop the media folder you want to pack. Make sure that the media folder only contains the NPK you want to repack, otherwise you might have a file that contains both images and scripts! After that, please rename the media~.npk file into your NPK, so the game can read it.
    
  <img width="430" height="101" alt="image" src="https://github.com/user-attachments/assets/9f61afa6-4750-4ef9-82ae-947a5962ee6d" />

The program contains a NUT file dialogue patcher, making editing text as easy as opening a text editor (recommended NotePad++) and setting the encoding to UTF8 (depending on the game, your encoding might need to be different.) No need to change the actual hex code of the NUT file manually, since the program automatically does it during repacking.


Plans for the future:
  - add automatic game detection
  - UI

If any errors or bugs are encountered, or even just for suggestions, please create an Issue, and I will try and fix it right away or tell you if you did something wrong.
If you use the MwareKeyFinder.dll on any Nitroplus game which is not supported yet, please post the decryption key and NPK version found in the Issues section of this repository!
