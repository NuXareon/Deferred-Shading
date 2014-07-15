#ifndef DEFERREDSHADING_H
#define DEFERREDSHADING_H

#include <QMainWindow>

class QSlider;

namespace Ui {
class DeferredShading;
}

class DeferredShading : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeferredShading(QWidget *parent = 0);
    ~DeferredShading();

private:
    Ui::DeferredShading *ui;
    QSlider *createSlider();
};

#endif // DEFERREDSHADING_H
