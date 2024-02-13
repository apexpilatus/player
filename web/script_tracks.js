function showtracks() {
    parent.document.getElementById("tracks").hidden = false;
}

function play(starting_track) {
    elem = document.getElementById("rate");
    elem.style.color="gray";
    fetch("play?" + starting_track)
    .then(x => x.status)
    .then(y => {
    if (y==200){
    elem.style.color="green";
    } else {
    elem.style.color="red"
    }})
}
