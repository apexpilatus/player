const controlElem = parent.document.getElementById("control");
const scrollupElem = parent.document.getElementById("scrollup");
const scrolldownElem = parent.document.getElementById("scrolldown");
const selectedalbumElem = parent.document.getElementById("selectedalbum");
const topalbumElem = parent.document.getElementById("topalbum");
const playerElem = parent.document.getElementById("player");
const iconElem = parent.document.getElementById("icon");
let empty = false;

function updatetop(album) {
    topalbumElem.innerHTML = album;
}

function gettracks(album) {
    if (selectedalbumElem.innerHTML == album) {
        fetch(location.origin + "/playflac?" + album).then(resp => {
            if (resp.status != 200) {
                if (iconElem.href != location.origin + album)
                    parent.location = location.origin + "/inbrowser?" + album
                playerElem.src = location.origin + "/stream_album?" + album;
                playerElem.play();
            } else {
                if (iconElem.title != "default")
                    parent.location = location.origin
                if (topalbumElem.innerHTML != album)
                    location.assign(location.origin + "/albums?up");
            }
        });
    } else {
        if (playerElem.title == autoplay) {
            playerElem.play();
            playerElem.title = "manual"
        }
        controlElem.hidden = true;
        controlElem.src = location.origin + "/tracks?" + album;
    }
}

function hidescroll() {
    empty = true;
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
}

function showscroll(initscroll) {
    if (!empty) {
        if (initscroll == "down")
            window.scrollTo(0, document.body.scrollHeight);
        scrollupElem.hidden = false;
        scrolldownElem.hidden = false;
    }
}