# New & Better Clone Thing

![Thumbnail](https://github.com/user-attachments/assets/155cefd5-bdbc-43d5-b2a0-3fdb65e4f242)

## Updates
Here are all the improvements compared to the previous version:

1. **Sleeker Design**: 40% thinner than the first version for a more compact look.
2. **Tool-Free Assembly**: SnapFit design allows for easy assembly without tools.
3. **Less Wiring Hassle**: A custom PCB simplifies assembly and reduces soldering.
4. **Simplified Controls**: No extra buttons—everything is controlled via the rotary encoder.
5. **New Video**: A detailed walkthrough of the new version! Watch here: [Clone Thing Made Easier](https://youtu.be/O8BWIcywnc4)

---

## Case
Check out the new and improved case design:

[![Case Preview](https://github.com/user-attachments/assets/a90b15b2-c142-4d17-b0d7-63361946f0e1)](https://makerworld.com/en/models/1159978-clone-thing#profileId-1165920)

🔗 [Download the Case Design](https://makerworld.com/en/models/1159978-clone-thing#profileId-1165920)

---

## Spotify API Setup
To set up your **Spotify API** credentials, follow these steps:

1. Log in to [Spotify for Developers](https://developer.spotify.com/dashboard/applications).
2. Click **Create New App**.
3. Enter any **Name** and **Description** you prefer.
4. Add a temporary **Redirect URI** (can be updated later).
5. Choose **Web API** as the application type.
6. Accept the required terms and conditions.
7. Click **Save**.
8. Navigate to **Settings**.
9. Click **Edit** and update the **Redirect URI** with the one displayed by your device.

---
## Driver Setup
For the display to work properly, we need to configure the drivers too:

1. For Windows: Navigate to "Documents\Arduino\libraries\TFT_eSPI" and edit User_Setup_Select.h.
    1a. For MacOS: Navigate in Finder to ~/Documents/Arduino/libraries/TFT_eSPI.
2. Comment the default library adding "//" in front of the #, then uncomment #include <User_Setups/Setup18_ST7789.h>. Save and exit.
    2a. If you are using a different display, then uncomment the setup related to that IC (eg. #include <User_Setups/Setup2_ST7735.h>)
3. Navigate to "Documents\Arduino\libraries\TFT_eSPI\User_Setups" and edit Setup18_ST7789.h
4. Uncomment '#define TFT_RGB_ORDER TFT_RGB' and comment out #define TFT_RGB_ORDER TFT_BGR
5. OPTIONAL: If your display is showing inverted colors consider switching between #define TFT_INVERSION_ON or #define TFT_INVERSION_OFF, also in the Clone_thing.ino file try swaping TJpgDec.setSwapBytes(true); with TJpgDec.setSwapBytes(false);
6. The drivers should be ready, have fun with your very Own-Clone Thing.

### You can also just replace the drivers with the files provided in the repo 
   
---

## Controls
- **1x Press** → Play/Pause
- **2x Press** → Next Song
- **3x Press** → Previous Song

---

## Old Version

### Wiring Diagram

![Wiring Diagram](https://github.com/user-attachments/assets/b4e44b36-2615-4fc2-8501-6ba9b88f5ee8)

---
