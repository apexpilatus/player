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
        void albumPage(@RequestParam("album") String album, HttpServletResponse resp, Storage store)
                        throws IOException {
                resp.setContentType("text/html");
                resp.setCharacterEncoding("UTF-8");
                PrintWriter respWriter = resp.getWriter();
                respWriter.println("<!DOCTYPE html>");
                respWriter.println("<html>");
                respWriter.println("<head>");
                respWriter.println("<meta charset=UTF-8>");
                respWriter.println("<link rel=stylesheet href=albumstyle.css>");
                respWriter.println("<script src=album.js></script>");
                respWriter.println("</head>");
                respWriter.println("<body>");
                respWriter.println("<script>gettrackspicture(\"" + album.replace(" ", "&") + "\")</script>");
                respWriter.println("<div>");
                File albumDirPath = new File(album);
                String[] files = albumDirPath.list();
                Arrays.sort(Objects.requireNonNull(files));
                StringBuilder title = new StringBuilder("<!DOCTYPE html><html><head><meta charset=UTF-8></head>\n");
                title.append("<body style=background-color:lightgray;>\n");
                for (String file : files) {
                        Map<String, String> metasMap = store.getMetas(album + "/" + file);
                        if (file.equals("01.flac")) {
                                title.append("<b style=color:black;font-size:120%;>")
                                                .append(metasMap.get("ARTIST"))
                                                .append("</b><br><strong style=color:slategray;>")
                                                .append(metasMap.get("ALBUM"))
                                                .append("</strong><b style=color:red;font-size:80%;> "
                                                                + metasMap.get("RATE") + "</b>\n");
                                title.append("</body></html>");
                        }
                        respWriter.println(
                                        "<i onclick=play(\"" + album.replace(" ", "&") + "\",\"" + file
                                                        + "\")><small style=color:white;>"
                                                        + metasMap.get("TRACKNUMBER") + "</small>"
                                                        + metasMap.get("TITLE") + "</i><br>");
                }
                respWriter.println("</div>");
                respWriter.println("<iframe src=\"data:text/html," + title + "\"></iframe>\n");
                respWriter.println("</body>");
                respWriter.println("</html>");
        }

        @PostMapping("/album")
        void albumPicture(@RequestParam("album") String album, HttpServletResponse resp, Storage store)
                        throws IOException, NoSuchFieldException, IllegalAccessException {
                resp.setContentType("image/jpeg");
                resp.getOutputStream().write(Base64.getEncoder().encode(store.getPictureBytes(album)));
        }
}
