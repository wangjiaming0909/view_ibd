#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    scene.addText("hello world");
    QGraphicsView view(&scene);
    view.show();
    return a.exec();
}
