package controllers;

import beans.Ipc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class Volume {
    @GetMapping("/volume")
    void setVolume(@RequestParam("direction") String volChangeDirection, HttpServletResponse resp, Ipc ipc) throws IOException {
        int vol = ipc.action2GetVol();
        if (vol != -1){
            switch (volChangeDirection) {
                case "up" -> vol++;
                case "down" -> vol--;
                default -> {
                }
            }
        }
        resp.setContentType("text/plain");
        resp.getWriter().write(String.valueOf(vol));
    }
}
