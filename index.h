const char mainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <title>Spotify Controller</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f4f4f9;
                color: #333;
                margin: 0;
                padding: 0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
                text-align: center;
            }
            .container {
                padding: 20px;
                background: #ffffff;
                border-radius: 10px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            }
            h1 {
                font-size: 1.8em;
                margin-bottom: 20px;
                color: #1db954;
            }
            a {
                display: inline-block;
                margin-top: 20px;
                padding: 10px 20px;
                background: #1db954;
                color: #ffffff;
                text-decoration: none;
                border-radius: 5px;
                font-size: 1em;
            }
            a:hover {
                background: #14833b;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Welcome to Spotify Controller</h1>
            <p>Control your music directly from this ESP-8266 project.</p>
            <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=user-modify-playback-state user-read-currently-playing user-read-playback-state user-library-modify user-library-read">
                Log in to Spotify
            </a>
        </div>
    </body>
</html>
)=====";

const char errorPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <title>Spotify Controller - Error</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #ffe6e6;
                color: #333;
                margin: 0;
                padding: 0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
                text-align: center;
            }
            .container {
                padding: 20px;
                background: #ffffff;
                border-radius: 10px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            }
            h1 {
                font-size: 1.8em;
                margin-bottom: 20px;
                color: #d9534f;
            }
            a {
                display: inline-block;
                margin-top: 20px;
                padding: 10px 20px;
                background: #d9534f;
                color: #ffffff;
                text-decoration: none;
                border-radius: 5px;
                font-size: 1em;
            }
            a:hover {
                background: #b52a2a;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Error Occurred</h1>
            <p>Something went wrong. Please try logging in again.</p>
            <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=user-modify-playback-state user-read-currently-playing user-read-playback-state user-library-modify user-library-read">
                Log in to Spotify
            </a>
        </div>
    </body>
</html>
)=====";

