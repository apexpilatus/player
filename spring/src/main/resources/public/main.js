function play(album) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        hideTracks();
    }
    xhttp.open("GET", window.location.href + "play?album=" + album.replace(/&/g, " "));
    xhttp.send();
}

function getTracks(album) {
    document.getElementById("tracks").src = window.location.href + "album?album=" + album.replace(/&/g, " ");
    document.getElementById("volume").hidden = true;
    document.getElementById("albums").setAttribute("class", "unscroll");
}

function hideTracks() {
    document.getElementById("albums").setAttribute("class", "scroll");
    document.getElementById("tracks").hidden = true;
    document.getElementById("tracks").src = window.location.href + "volume";
    document.getElementById("trackspicture").hidden = true;
    document.getElementById("hidetracks").hidden = true;
}

function getVolume() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        resp = this.responseText;
        if (resp.split(";")[1] == 0) {
            document.getElementById("showvolume").innerHTML = "&#9739";
        } else {
            hideTracks();
            document.getElementById("showvolume").innerHTML = "&#9738";
            document.getElementById("volume").min = 0;
            document.getElementById("volume").max = resp.split(";")[1];
            document.getElementById("volume").value = resp.split(";")[0];
            document.getElementById("volume").hidden = false;
            document.getElementById("showvolume").hidden = true;
            delayVar = setTimeout(function () {
                document.getElementById("volume").hidden = true;
                document.getElementById("showvolume").hidden = false;
            }, 4000);
        }
    }
    xhttp.open("GET", window.location.href + "volume");
    xhttp.send();
}

function setVolume() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        clearTimeout(delayVar);
        delayVar = setTimeout(function () {
            document.getElementById("volume").hidden = true;
            document.getElementById("showvolume").hidden = false;
        }, 4000);
    }
    xhttp.open("POST", window.location.href + "volume?level=" + document.getElementById("volume").value);
    xhttp.send();
}
