function getalbums() {
    document.getElementById("albums").src = window.location.href + "albums";
}

function getvolume() {
    fetch("getvolume")
        .then(resp => {
            if (resp.status == 200) {
                document.getElementById("volume").innerHTML = "&#9738";
                document.getElementById("control").hidden = false;
            } else {
                document.getElementById("volume").innerHTML = "&#9739";
            }
        })
}
