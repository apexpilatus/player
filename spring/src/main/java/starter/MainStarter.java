package starter;

import controllers.AlbumPage;
import controllers.MainPage;
import controllers.Play;
import controllers.Volume;
import configuration.Config;
import org.springframework.boot.Banner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Import;

@SpringBootApplication
@Import({ MainPage.class, Volume.class, AlbumPage.class, Play.class, Config.class })
public class MainStarter {
    public static void main(String[] args) {
        SpringApplication application = new SpringApplication(MainStarter.class);
        application.setBannerMode(Banner.Mode.OFF);
        application.run(args);
    }
}