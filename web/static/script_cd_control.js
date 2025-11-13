const controlElem = parent.document.getElementById("control");
const playerElem = parent.document.getElementById("player");

function showtracks() {
    controlElem.hidden = false;
}

function playcd(track){
fetch(parent.window.location.href + "playcd?" + track).then(resp => {
if (resp.status != 200)
	playerElem.src = parent.window.location.href + "stream_cd?" + track;

});
}
