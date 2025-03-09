const tracksElem = parent.document.getElementById("tracks");
const currentElem = parent.document.getElementById("current");

function showtracks() {
    tracksElem.hidden = false;
}

function updatecurrent() {
    currentElem.innerHTML="cd";
}
