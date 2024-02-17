function showtracks() {
    parent.document.getElementById("tracks").hidden = false;
}

function play(startingtrack) {
    let elem = document.getElementById("rate");
    elem.style.color = "gray";
    fetch("play?" + startingtrack)
        .then(resp => {
            if (resp.status == 200) {
                elem.style.color = "green";
            } else {
                elem.style.color = "red";
            }
        })
}
