#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.

#include <GLFW/glfw3.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include "wowScene.h"
#include <zip.h>

#include "persistance/httpFile/httpFile.h"
int mleft_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentContollable();

//    if (!pointerIsLocked) {
        if (mleft_pressed == 1) {
            controllable->addHorizontalViewDir((xpos - m_x) / 4.0);
            controllable->addVerticalViewDir((ypos - m_y) / 4.0);

            m_x = xpos;
            m_y = ypos;
        }
//    } else {
//        var delta_x = event.movementX ||
//                      event.mozMovementX          ||
//                      event.webkitMovementX       ||
//                      0;
//        var delta_y = event.movementY ||
//                      event.mozMovementY      ||
//                      event.webkitMovementY   ||
//                      0;
//
//        camera.addHorizontalViewDir((delta_x) / 4.0);
//        camera.addVerticalViewDir((delta_y) / 4.0);
//    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mleft_pressed = 1;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        m_x = xpos;
        m_y = ypos;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mleft_pressed = 0;
    }
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentContollable();
    if ( action == GLFW_PRESS) {
        switch (key) {
            case 'W' :
                controllable->startMovingForward();
                break;
            case 'S' :
                controllable->startMovingBackwards();
                break;
            case 'A' :
                controllable->startStrafingLeft();
                break;
            case 'D':
                controllable->startStrafingRight();
                break;
            case 'Q':
                controllable->startMovingUp();
                break;
            case 'E':
                controllable->startMovingDown();
                break;

            default:
                break;
        }
    } else if ( action == GLFW_RELEASE) {
        switch (key) {
            case 'W' :
                controllable->stopMovingForward();
                break;
            case 'S' :
                controllable->stopMovingBackwards();
                break;
            case 'A' :
                controllable->stopStrafingLeft();
                break;
            case 'D':
                controllable->stopStrafingRight();
                break;
            case 'Q':
                controllable->stopMovingUp();
                break;
            case 'E':
                controllable->stopMovingDown();
                break;

            default:
                break;
        }
    }

}

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}

class RequestProcessor : public IFileRequest {
public:
    RequestProcessor () {
        char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";

        using namespace std::placeholders;
        HttpFile httpFile(new std::string(url));
        httpFile.setCallback(std::bind(&RequestProcessor::loadingFinished, this, _1));
        httpFile.startDownloading();
    }
private:
//    zipper::Unzipper *m_unzipper;
    zip_t *zipArchive;
    IFileRequester *m_fileRequester;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
    void loadingFinished(std::vector<unsigned char> * file) {
        zip_source_t *src;
        zip_t *za;
        zip_error_t error;

        zip_error_init(&error);
        /* create source from buffer */
        if ((src = zip_source_buffer_create(&file->at(0), file->size(), 1, &error)) == NULL) {
            fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
//            free(data);
            zip_error_fini(&error);
//            return 1;
        }

        /* open zip archive from source */
        if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
            fprintf(stderr, "can't open zip from source: %s\n", zip_error_strerror(&error));
            zip_source_free(src);
            zip_error_fini(&error);
//            return 1;
        }
        zip_error_fini(&error);
        zipArchive = za;

        /* we'll want to read the data back after zip_close */
//        zip_source_keep(src);
    }

    void requestFile(const char* fileName) {
        std::string s_fileName(fileName);
        zip_error_t error;
        struct zip_stat sb;
        zip_file *zf;

        zip_int64_t record = zip_name_locate(zipArchive, fileName, ZIP_FL_NOCASE);

//        for (int i = 0; i < m_unzipper->entries().size(); i++) {
//            auto entry = m_unzipper->entries().at(i);
//            if (strcicmp(entry.name.c_str(), fileName) ==0){
//                s_fileName = entry.name;
//                break;
//            }
//        }

//        std::vector<unsigned char> unzipped_entry;
//        if (m_unzipper->extractEntryToMemory(s_fileName, unzipped_entry)) {
//            m_fileRequester->provideFile(fileName, &unzipped_entry[0], unzipped_entry.size());
//        } else {
//            m_fileRequester->rejectFile(fileName);
        if (record != -1) {
            if (zip_stat_index(zipArchive, record, 0, &sb) != 0) {
                throw "errror";
            }
            zf = zip_fopen_index(zipArchive, record, 0);

            unsigned char *unzippedEntry = new unsigned char[sb.size];

            int sum = 0;
            while (sum != sb.size) {
                zip_int64_t len = zip_fread(zf, &unzippedEntry[sum], 1024);
                if (len < 0) {
                    //error
                    exit(102);
                }
                sum += len;
            }

            zip_fclose(zf);


            m_fileRequester->provideFile(fileName, unzippedEntry, sum);
            delete(unzippedEntry);
        } else {
            m_fileRequester->rejectFile(fileName);
        }
    };
};


int main(int argc, char** argv) {
    CURL *curl = NULL;
    FILE *fp;
    CURLcode res;


    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

//     Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    window = glfwCreateWindow( 1024, 768, "Test Window", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW

    Config *testConf = new Config();
    RequestProcessor *processor = new RequestProcessor();
    WoWScene *scene = createWoWScene(testConf, processor, 1000, 1000);
    processor->setFileRequester(scene);

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowUserPointer(window, scene);
    glfwSetKeyCallback(window, onKey);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    double deltaTime;
    do {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        scene->draw(deltaTime*1000);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

//    } while(true);// Check if the ESC key was pressed or the window was closed
    }    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    return 0;
}