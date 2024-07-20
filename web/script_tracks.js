function showtracks() {
    parent.document.getElementById("tracks").hidden = false;
}

function play(dirtrack) {
    let elem = document.getElementById("rate");
    elem.style.color = "gray";
    fetch("play?" + dirtrack);
}
