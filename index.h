const char mainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Spotify Controller</title>
</head>
<body>
    <h1>Spotify Controller</h1>
    <p>Control your music from this ESP-8266 project.</p>
    <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=user-modify-playback-state user-read-currently-playing user-read-playback-state user-library-modify user-library-read">
        Log in to Spotify
    </a>
</body>
</html>
)=====";

const char errorPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Error</title>
</head>
<body>
    <h1>Error</h1>
    <p>Something went wrong. Please try again.</p>
    <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=user-modify-playback-state user-read-currently-playing user-read-playback-state user-library-modify user-library-read">
        Log in to Spotify
    </a>
</body>
</html>
)=====";

