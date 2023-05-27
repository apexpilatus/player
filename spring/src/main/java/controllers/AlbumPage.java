package controllers;

import beans.Storage;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.Objects;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage(@RequestParam("album") String album, HttpServletResponse resp, Storage store) throws IOException {
        resp.setContentType("text/html");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        PrintWriter respWriter = resp.getWriter();
        respWriter.println("<!DOCTYPE html>");
        respWriter.println("<html>");
        respWriter.println("<head>");
        respWriter.println("<meta name=viewport content=\"width=device-width, initial-scale=1.0\" charset=utf-8>");
        respWriter.println("<link rel=stylesheet href=albumstyle.css>");
        respWriter.println("<script src=album.js></script>");
        respWriter.println("</head>");
        respWriter.println("<body>");
        respWriter.println("<script>gettrackspicture(\"" + album.replace(" ", "&") + "\")</script>");
        File albumDirPath = new File(album);
        String[] files = albumDirPath.list();
        Arrays.sort(Objects.requireNonNull(files));
        StringBuilder title = new StringBuilder();
        StringBuilder tracks = new StringBuilder();
        for (String file : files) {
            Map<String, String> metasMap = store.getMetas(album + "/" + file);
            if (file.equals("01.flac")) {
                title.append("<div class=artist>").append(metasMap.get("ARTIST")).append("</div>\n").append("<div class=album>").append(metasMap.get("ALBUM")).append("</div>\n").append("<div class=rate> ").append(metasMap.get("RATE")).append("</div>");
            }
            tracks.append("<div onclick=play(\"").append(album.replace(" ", "&")).append("\",\"").append(file).append("\")>").append("<small>").append(metasMap.get("TRACKNUMBER")).append("</small>").append("<b class=track>").append(metasMap.get("TITLE")).append("</b></div>");
        }
        respWriter.println("<div class=title>");
        respWriter.println(title);
        respWriter.println("</div>");
        respWriter.println("<div class=tracks>");
        respWriter.println(tracks);
        respWriter.println("</div>");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }

    @PostMapping("/album")
    void albumPicture(@RequestParam("album") String album, HttpServletResponse resp, Storage store) throws IOException, NoSuchFieldException, IllegalAccessException {
        resp.setContentType("image/jpeg");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        resp.getOutputStream().write(Base64.getEncoder().encode(store.getPictureBytes(album)));
    }
}
