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
    if (iconElem.title == album && playerElem.src == "") {
        playerElem.src = parent.window.location.href + "stream_album?" + album;
        controlElem.src = parent.window.location.href + "tracks?" + album + "&full";
    }
    else if (selectedalbumElem.innerHTML == album)
        fetch(parent.window.location.href + "playflac?" + album).then(resp => {
            if (resp.status != 200) {
                if (iconElem.title == album)
                    playerElem.src = parent.window.location.href + "stream_album?" + album;
                else
                    parent.window.location.assign(parent.window.location.href)
            }
            else if (topalbumElem.innerHTML != album)
                window.location.assign(parent.window.location.href + "albums?up");
        });
    else {
        controlElem.hidden = true;
        controlElem.src = parent.window.location.href + "tracks?" + album;
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