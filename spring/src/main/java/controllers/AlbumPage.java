package controllers;

import beans.MetaIpc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Base64;
import java.util.Map;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage(@RequestParam("album") String album, HttpServletResponse resp, MetaIpc metaIpc) throws IOException {
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
        StringBuilder title = new StringBuilder();
        StringBuilder tracks = new StringBuilder();
        Map<String, Map<String, String>> metasMap = metaIpc.meta2GetTags(album);
        metasMap.forEach((file, meta) -> {
            if (file.equals("01.flac")) {
                title.append("<div class=artist>").append(meta.get("ARTIST")).append("</div>\n").append("<div class=album>").append(meta.get("ALBUM")).append("</div>\n").append("<div class=rate>").append(meta.get("RATE")).append("</div>");
            }
            tracks.append("<tr onclick=play(\"").append(album.replace(" ", "&")).append("\",\"").append(file).append("\")>").append("<td class=tracknumber>").append(meta.get("TRACKNUMBER")).append("</td>").append("<td class=tracktitle>").append(meta.get("TITLE")).append("</td>").append("</tr>");
        });
        respWriter.println("<div class=title>");
        respWriter.println(title);
        respWriter.println("</div>");
        respWriter.println("<table>");
        respWriter.println(tracks);
        respWriter.println("</table>");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }

    @PostMapping("/album")
    void albumPicture(@RequestParam("album") String album, HttpServletResponse resp, MetaIpc metaIpc) throws IOException {
        resp.setContentType("image/jpeg");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        resp.getOutputStream().write(Base64.getEncoder().encode(metaIpc.meta1GetPicture(album)));
    }
}
