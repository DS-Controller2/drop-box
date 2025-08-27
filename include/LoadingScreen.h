#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

class LoadingScreen : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    void setMessage(const QString &message);
    void setProgress(int value);

private:
    QLabel *messageLabel;
    QProgressBar *progressBar;
};

#endif // LOADINGSCREEN_H
