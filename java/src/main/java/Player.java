import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.*;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Objects;

public class Player extends HttpServlet {
    String musicPath = "/home/store/music";
    String exeDirPath = "/home/exe";
    String albumFilePath = exeDirPath + "/player/tmp/album";
    String volumeFilePath = exeDirPath + "/player/tmp/volume";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        String volChangeDirection = req.getParameter("volume");
        File volFile = new File(volumeFilePath);
        if (volChangeDirection != null) {
            int vol = 5;
            if (volFile.exists()) {
                try (BufferedReader volumeFileReader = new BufferedReader(new FileReader(volumeFilePath))) {
                    vol = volumeFileReader.read();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            switch (volChangeDirection) {
                case "up":
                    vol++;
                    break;
                case "down":
                    vol--;
                    break;
                default:
            }
            resp.setContentType("text/plain");
            try (BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(volumeFilePath))) {
                albumFileWriter.write(vol);
                resp.getWriter().println(vol);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        File musicDir = new File(musicPath);
        if (musicDir.exists() && musicDir.isDirectory()) {
            String albumToPlay = req.getParameter("album");
            if (albumToPlay != null) {
	    	String picturePath = "picture.jpeg";
	    	File exeDirFile = new File(exeDirPath);
		for (String file : exeDirFile.list()) {
			if (file.contains("apache-tom")) {
				picturePath = exeDirPath + "/" + file + "/" + "webapps/ROOT/picture.jpeg";
			}
		}
		System.out.println("fuck");
		System.out.println(picturePath);
		System.out.println("fuck");
                try (
                        BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(albumFilePath));
                        FileInputStream flacIs = new FileInputStream(albumToPlay + "/01.flac");
                        FileOutputStream pictureOs = new FileOutputStream(picturePath)) {
                    albumFileWriter.write(albumToPlay);
                    FLACDecoder flacDec = new FLACDecoder(flacIs);
                    Metadata[] metas = flacDec.readMetadata();
                    for (Metadata meta : metas) {
                        if (meta.toString().contains("Picture")) {
                            Picture picMeta = (Picture) meta;
                            Class<? extends Picture> c = picMeta.getClass();
                            Field f = c.getDeclaredField("image");
                            f.setAccessible(true);
                            byte[] picture = (byte[]) f.get(picMeta);
                            pictureOs.write(picture);
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            String[] albums = musicDir.list();
            Arrays.sort(Objects.requireNonNull(albums));
            if (Objects.requireNonNull(albums).length != 0) {
                resp.setContentType("text/html");
                try {
                    resp.getWriter().println("<iframe name=vol height=45px width=50px style=border:none;position:fixed;top:40;right:60;></iframe>");
                    resp.getWriter().println("<body style=\"background-color:gray\">");
                    resp.getWriter().println("<form action=http://" + req.getHeader("Host") +
                            "/player?volume=up method=post target=vol style=position:fixed;top:40;left:20;>");
                    resp.getWriter().println("<input type=submit value=up>");
                    resp.getWriter().println("</form>");
                    resp.getWriter().println("<form action=http://" + req.getHeader("Host") +
                            "/player?volume=down method=post target=vol style=position:fixed;top:100;left:20;>");
                    resp.getWriter().println("<input type=submit value=dw>");
                    resp.getWriter().println("</form>");
                    for (String album : albums) {
                        resp.getWriter().println("<p style=text-align:center;color:white;font-size:150%><a href=http://" + req.getHeader("Host") +
                                "/player?album=" + musicPath + "/" + album.replace(" ", "%20") + ">" +
                                album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace(" anD ", " & ").replace("___", "</a> <b><small>") +
                                "</a></b></small></p>");
                    }
                    resp.getWriter().println("<p style=\"font-size:1em;text-align:center\">_ _ _ _ _ _ _</p>");
                    resp.getWriter().println("</body>");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
