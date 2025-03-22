const controlElem = parent.document.getElementById("control");
const scrollupElem = parent.document.getElementById("scrollup");
const scrolldownElem = parent.document.getElementById("scrolldown");
const currentElem = parent.document.getElementById("current");
const topElem = parent.document.getElementById("top");

function updatetop(album) {
    topElem.innerHTML = album;
}

function gettracks(album) {
    if (currentElem.innerHTML == album) {
        fetch("playflac?" + album).then(resp => {
            if (resp.status == 200 && topElem.innerHTML != album) {
                window.location.assign(parent.window.location.href + "albums");
            }
        });
    } else {
        controlElem.hidden = true;
        controlElem.src = parent.window.location.href + "tracks?" + album;
    }
}

function showscroll(initscroll) {
    if (initscroll == "down")
        window.scrollTo(0, document.body.scrollHeight);
    scrollupElem.hidden = false;
    scrolldownElem.hidden = false;
}