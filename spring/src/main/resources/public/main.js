function play(album, rate) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        hideTracks();
    }
    xhttp.open("GET", window.location.href + "play?album=" + album.replace(/&/g, " ") + "&rate=" + rate);
    xhttp.send();
}

function getTracks(album) {
    document.getElementById("tracks").src = window.location.href + "album?album=" + album.replace(/&/g, " ");
    document.getElementById("showvolume").hidden = true;
    document.getElementById("albums").setAttribute("class", "unscroll");
}

function hideTracks() {
    document.getElementById("tracks").src = "data:text/plain,ok";
    document.getElementById("albums").setAttribute("class", "scroll");
    document.getElementById("tracks").hidden = true;
    document.getElementById("trackspicture").hidden = true;
    document.getElementById("hidetracks").hidden = true;
    document.getElementById("showvolume").hidden = false;
}

function hideVolume() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        document.getElementById("volume").hidden = true;
        document.getElementById("showvolume").hidden = false;
        document.getElementById("albums").setAttribute("class", "scroll");
    }
    xhttp.open("GET", window.location.href + "volume?close=true");
    xhttp.send();
}

function getVolume() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        resp = this.responseText;
        if (resp.split(";")[1] == 0) {
            document.getElementById("showvolume").innerHTML = "&#9739";
            hideVolume();
        } else {
            document.getElementById("albums").setAttribute("class", "unscroll");
            document.getElementById("showvolume").innerHTML = "&#9738";
            document.getElementById("volume").min = 0;
            document.getElementById("volume").max = resp.split(";")[1];
            document.getElementById("volume").value = resp.split(";")[0];
            document.getElementById("volume").hidden = false;
            document.getElementById("showvolume").hidden = true;
            delayVar = setTimeout(hideVolume, 3000);
        }
    }
    xhttp.open("GET", window.location.href + "volume");
    xhttp.send();
}

function setVolume() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        clearTimeout(delayVar);
        delayVar = setTimeout(hideVolume, 3000);
    }
    xhttp.open("POST", window.location.href + "volume?level=" + document.getElementById("volume").value);
    xhttp.send();
}
