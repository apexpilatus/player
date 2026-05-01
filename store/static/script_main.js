
const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");

function playflac(dirtrack) {
    fetch(location.origin + "/touch?" + dirtrack.split("&")[0]).then(resp => {
        if (resp.status == 200) {
            playerElem.src = location.origin + "/stream_album?" + dirtrack;
            if (topalbumElem.innerHTML != dirtrack.split("&")[0])
                albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}