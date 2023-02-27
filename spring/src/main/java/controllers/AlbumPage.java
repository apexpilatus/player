package controllers;

import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Objects;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage(@RequestParam("album") String album, HttpServletResponse resp) throws IOException {
        resp.setContentType("text/html");
        resp.setCharacterEncoding("UTF-8");
        PrintWriter respWriter = resp.getWriter();
        respWriter.println("<!DOCTYPE html>");
        respWriter.println("<html>");
        respWriter.println("<head><meta charset=UTF-8></head>");
        respWriter.println("<body style=background-color:gray;>");
        File albumDirPath = new File(album);
        String[] files = albumDirPath.list();
        Arrays.sort(Objects.requireNonNull(files));
        StringBuilder title = new StringBuilder("<!DOCTYPE html><html><head><meta charset=UTF-8></head>\n");
        title.append("<body style=background-color:lightgray;>\n");
        for (String file : files) {
            try (FileInputStream flacIs = new FileInputStream(album + "/" + file)) {
                FLACDecoder flacDec = new FLACDecoder(flacIs);
                Metadata[] metas = flacDec.readMetadata();
                final String[] vorbisTrack = {""};
                final String[] vorbisTitle = {""};
                final String[] vorbisAlbum = {""};
                final String[] vorbisArtist = {""};
                for (Metadata meta : metas) {
                    if (meta.toString().contains("VorbisComment")) {
                        meta.toString().lines().forEach((line) -> {
                            if (line.contains("TRACKNUMBER")) {
                                vorbisTrack[0] = line.split("=")[1] + ". ";
                            }
                            if (line.contains("TITLE")) {
                                vorbisTitle[0] = line.substring(7);
                            }
                            if (file.equals("01.flac") && line.contains("ALBUM")) {
                                vorbisAlbum[0] = line.substring(7);
                            }
                            if (file.equals("01.flac") && line.contains("ARTIST")) {
                                vorbisArtist[0] = line.substring(8);
                            }
                        });
                    }
                }
                if (file.equals("01.flac")) {
                    title.append("<p style=color:black;font-size:120%;><b>").append(vorbisArtist[0]).append("</b><br><strong style=color:slategray;>").append(vorbisAlbum[0]).append("</strong></p>\n");
                    title.append("</body></html>");
                }
            } catch (IOException e){
                throw new RuntimeException(e);
            }
        }
        respWriter.println("<iframe height=115 width=450 style=position:fixed;top:0px;left:0px;border:none src=\"data:text/html," + title + "\"></iframe>\n");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }
}