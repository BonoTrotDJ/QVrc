/*
  Q Light Controller Plus
  videoprovider.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QVersionNumber>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
 #include <QMediaMetaData>
 #include <QAudioOutput>
#endif
#include <QApplication>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QScreen>
#include <QToolButton>
#include <QEvent>
#include <QUrl>

#include "videoprovider.h"
#include "qlcfile.h"
#include "doc.h"

VideoProvider::VideoProvider(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    connect(m_doc, SIGNAL(functionAdded(quint32)),
            this, SLOT(slotFunctionAdded(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

VideoProvider::~VideoProvider()
{
    shutdown();
}

void VideoProvider::shutdown()
{
    for (VideoWidget *vw : std::as_const(m_videoMap))
    {
        if (vw != NULL)
            vw->shutdown();
    }

    qDeleteAll(m_videoMap);
    m_videoMap.clear();
}

void VideoProvider::slotFunctionAdded(quint32 id)
{
    Function *func = m_doc->function(id);
    if (func == NULL)
        return;

    if (func->type() == Function::VideoType)
    {
        VideoWidget *vWidget = new VideoWidget(qobject_cast<Video *>(func));
        m_videoMap[id] = vWidget;
    }
}

void VideoProvider::slotFunctionRemoved(quint32 id)
{
    if (m_videoMap.contains(id))
    {
        VideoWidget *vw = m_videoMap.take(id);
        delete vw;
    }
}

/*********************************************************************
 * VideoWidget class implementation
 *********************************************************************/

VideoWidget::VideoWidget(Video *video, QObject *parent)
    : QObject(parent)
    , m_video(video)
    , m_videoPlayer(NULL)
    , m_videoWidget(NULL)
    , m_closeButton(NULL)
{
    Q_ASSERT(video != NULL);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_videoPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
#else
    m_videoPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_videoPlayer->setAudioOutput(m_audioOutput);
#endif
    m_videoPlayer->moveToThread(QCoreApplication::instance()->thread());

    if (QLCFile::getQtRuntimeVersion() >= 50700 && m_videoWidget == NULL)
    {
        m_videoWidget = new QVideoWidget;
        m_videoWidget->setStyleSheet("background-color:black;");
        m_videoPlayer->setVideoOutput(m_videoWidget);
        m_videoWidget->installEventFilter(this);
        ensureCloseButton();
    }

    connect(m_videoPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotStatusChanged(QMediaPlayer::MediaStatus)));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_videoPlayer, SIGNAL(metaDataChanged(QString,QVariant)),
            this, SLOT(slotMetaDataChanged(QString,QVariant)));
#else
    connect(m_videoPlayer, SIGNAL(metaDataChanged()),
            this, SLOT(slotMetaDataChanged()));
#endif
    connect(m_videoPlayer, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotTotalTimeChanged(qint64)));

    connect(m_video, SIGNAL(sourceChanged(QString)),
            this, SLOT(slotSourceUrlChanged(QString)));
    connect(m_video, SIGNAL(requestPlayback()),
            this, SLOT(slotPlaybackVideo()));
    connect(m_video, SIGNAL(requestPause(bool)),
            this, SLOT(slotSetPause(bool)));
    connect(m_video, SIGNAL(requestStop()),
            this, SLOT(slotStopVideo()));
    connect(m_video, SIGNAL(requestBrightnessVolumeAdjust(qreal)),
            this, SLOT(slotBrightnessVolumeAdjust(qreal)));

    QString sourceURL = m_video->sourceUrl();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (sourceURL.contains("://"))
        m_videoPlayer->setMedia(QUrl(sourceURL));
    else
        m_videoPlayer->setMedia(QUrl::fromLocalFile(sourceURL));
#else
    if (sourceURL.contains("://"))
        m_videoPlayer->setSource(QUrl(sourceURL));
    else
        m_videoPlayer->setSource(QUrl::fromLocalFile(sourceURL));
#endif
    qDebug() << "Video source URL:" << sourceURL;
}

void VideoWidget::shutdown()
{
    if (m_videoPlayer != NULL)
        m_videoPlayer->stop();

    if (m_videoWidget != NULL)
    {
        if (m_video->fullscreen())
            m_videoWidget->setFullScreen(false);

        m_videoWidget->hide();
        m_videoWidget->close();
        m_videoWidget->deleteLater();
        m_videoWidget = NULL;
        m_closeButton = NULL;
    }

    m_video->stop(functionParent());
}

void VideoWidget::slotSourceUrlChanged(QString url)
{
    qDebug() << "Video source URL changed:" << url;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (url.contains("://"))
        m_videoPlayer->setMedia(QUrl(url));
    else
        m_videoPlayer->setMedia(QUrl::fromLocalFile(url));
#else
    if (url.contains("://"))
        m_videoPlayer->setSource(QUrl(url));
    else
        m_videoPlayer->setSource(QUrl::fromLocalFile(url));
#endif
}

void VideoWidget::slotTotalTimeChanged(qint64 duration)
{
    qDebug() << "Video duration: " << duration;
    m_video->setTotalDuration(duration);
}

void VideoWidget::slotStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << Q_FUNC_INFO << status;
    switch (status)
    {
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::BufferedMedia:
            //setStatusInfo(QString());
        break;
        case QMediaPlayer::LoadingMedia:
            //setStatusInfo(tr("Loading..."));
        break;
        case QMediaPlayer::StalledMedia:
            //setStatusInfo(tr("Media Stalled"));
        break;
        case QMediaPlayer::EndOfMedia:
        {
            if (m_videoPlayer != NULL)
                m_videoPlayer->stop();

            if (m_video->runOrder() == Video::Loop)
            {
                m_videoPlayer->play();
                break;
            }

            if (m_videoWidget != NULL)
                m_videoWidget->hide();

            m_video->stop(functionParent());
            break;
        }
        default:
        case QMediaPlayer::InvalidMedia:
            //displayErrorMessage();
        break;
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void VideoWidget::slotMetaDataChanged(QString key, QVariant data)
{
    if (m_video == NULL)
        return;

    qDebug() << Q_FUNC_INFO << "Got meta data:" << key;
    if (key == "Resolution")
        m_video->setResolution(data.toSize());
    else if (key == "VideoCodec")
        m_video->setVideoCodec(data.toString());
    else if (key == "AudioCodec")
        m_video->setAudioCodec(data.toString());
}
#else
void VideoWidget::slotMetaDataChanged()
{
    if (m_video == NULL)
        return;

    QMediaMetaData md = m_videoPlayer->metaData();
    foreach (QMediaMetaData::Key k, md.keys())
    {
        qDebug() << "[Metadata]" << md.metaDataKeyToString(k) << ":" << md.stringValue(k);
        switch (k)
        {
            case QMediaMetaData::Resolution:
                m_video->setResolution(md.value(k).toSize());
            break;
            case QMediaMetaData::VideoCodec:
                m_video->setVideoCodec(md.stringValue(k));
            break;
            case QMediaMetaData::AudioCodec:
                m_video->setAudioCodec(md.stringValue(k));
            break;
            default:
            break;
        }
    }
}
#endif

void VideoWidget::slotPlaybackVideo()
{
    // Always refresh player source from the current Video function state.
    const QString sourceURL = m_video->sourceUrl();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (sourceURL.contains("://"))
        m_videoPlayer->setMedia(QUrl(sourceURL));
    else
        m_videoPlayer->setMedia(QUrl::fromLocalFile(sourceURL));
#else
    if (sourceURL.contains("://"))
        m_videoPlayer->setSource(QUrl(sourceURL));
    else
        m_videoPlayer->setSource(QUrl::fromLocalFile(sourceURL));
#endif

    int screen = m_video->screen();
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen *scr = screens.count() > screen ? screens.at(screen) : screens.first();
    QRect rect = scr->availableGeometry();

    if (QLCFile::getQtRuntimeVersion() < 50700 && m_videoWidget == NULL)
    {
        m_videoWidget = new QVideoWidget;
        m_videoWidget->setStyleSheet("background-color:black;");
        m_videoPlayer->setVideoOutput(m_videoWidget);
        m_videoWidget->installEventFilter(this);
        ensureCloseButton();
    }

    m_videoWidget->setWindowFlags(m_videoWidget->windowFlags() | Qt::WindowStaysOnTopHint);
    if (m_video->fullscreen() == false)
    {
        QSize resolution = m_video->resolution();
        m_videoWidget->setFullScreen(false);
        if (resolution.isEmpty())
            m_videoWidget->setGeometry(0, 50, 640, 480);
        else
            m_videoWidget->setGeometry(0, 50, resolution.width(), resolution.height());
        m_videoWidget->move(rect.topLeft());
    }
    else
    {
#if defined(WIN32) || defined(Q_OS_WIN)
        m_videoWidget->setFullScreen(true);
        m_videoWidget->setGeometry(rect);
#else
        m_videoWidget->setGeometry(rect);
        m_videoWidget->setFullScreen(true);
#endif
    }

    if (m_videoPlayer->isSeekable())
        m_videoPlayer->setPosition(m_video->elapsed());
    else
        m_videoPlayer->setPosition(0);

    m_videoWidget->show();
    if (m_closeButton != NULL)
    {
        updateCloseButtonPosition();
        m_closeButton->show();
        m_closeButton->raise();
    }
    m_videoPlayer->play();
}

void VideoWidget::slotSetPause(bool enable)
{
    if (enable)
        m_videoPlayer->pause();
    else
        m_videoPlayer->play();
}

void VideoWidget::slotStopVideo()
{
    if (m_videoPlayer != NULL)
        m_videoPlayer->stop();

    if (m_videoWidget != NULL)
    {
        if (m_video->fullscreen())
            m_videoWidget->setFullScreen(false);
        m_videoWidget->hide();
    }

    m_video->stop(functionParent());
}

void VideoWidget::slotBrightnessVolumeAdjust(qreal value)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int brightness = -100 + (int)(qreal(100.0) * value);
    int volume = 100 * (int)QAudio::convertVolume(value, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
    if (m_videoWidget)
        m_videoWidget->setBrightness(brightness);
    if (m_videoPlayer)
        m_videoPlayer->setVolume(volume);
#else
    qreal linearVolume = QAudio::convertVolume(value, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
    if (m_audioOutput)
        m_audioOutput->setVolume(linearVolume);
#endif
}

int VideoWidget::getScreenCount()
{
    int screenCount = QGuiApplication::screens().count();

    return screenCount;
}

void VideoWidget::ensureCloseButton()
{
    if (m_videoWidget == NULL || m_closeButton != NULL)
        return;

    m_closeButton = new QToolButton(m_videoWidget);
    m_closeButton->setText("x");
    m_closeButton->setToolTip(tr("Close video"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(false);
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setStyleSheet("QToolButton { background-color: rgba(198, 40, 40, 220); color: white; border: 1px solid white; font-weight: bold; }");
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(slotStopVideo()));
    updateCloseButtonPosition();
    m_closeButton->hide();
}

void VideoWidget::updateCloseButtonPosition()
{
    if (m_videoWidget == NULL || m_closeButton == NULL)
        return;

    const int margin = 6;
    const int x = m_videoWidget->width() - m_closeButton->width() - margin;
    const int y = margin;
    m_closeButton->move(qMax(0, x), qMax(0, y));
}

bool VideoWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_videoWidget &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        updateCloseButtonPosition();
    }

    return QObject::eventFilter(watched, event);
}

FunctionParent VideoWidget::functionParent() const
{
    return FunctionParent::master();
}
