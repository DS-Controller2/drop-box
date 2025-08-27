#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(QWidget *parent) : QDialog(parent)
{
    setFixedSize(300, 150);
    setWindowTitle("Loading...");
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);

    QVBoxLayout *layout = new QVBoxLayout(this);

    messageLabel = new QLabel("Loading application...", this);
    messageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(messageLabel);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    layout->addWidget(progressBar);

    setLayout(layout);
}

void LoadingScreen::setMessage(const QString &message)
{
    messageLabel->setText(message);
}

void LoadingScreen::setProgress(int value)
{
    progressBar->setValue(value);
}
