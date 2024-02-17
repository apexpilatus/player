function getalbums() {
    document.getElementById("albums").src = window.location.href + "albums";
}

function getvolume() {
    fetch("getvolume")
        .then(resp => {
            if (resp.status == 200) {
                document.getElementById("volume").innerHTML = "&#9738";
                document.getElementById("control").min = resp.statusText.split("_")[0];
                document.getElementById("control").value = resp.statusText.split("_")[1];
                document.getElementById("control").max = resp.statusText.split("_")[2];
                document.getElementById("control").hidden = false;
            } else {
                document.getElementById("volume").innerHTML = "&#9739";
            }
        })
}
