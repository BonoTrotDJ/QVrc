/*
  Q Light Controller Plus
  decryptworkspacedialog.h

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

#ifndef DECRYPTWORKSPACEDIALOG_H
#define DECRYPTWORKSPACEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class DecryptWorkspaceDialog;
}

class DecryptWorkspaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecryptWorkspaceDialog(QWidget *parent = nullptr);
    ~DecryptWorkspaceDialog();

public slots:
    void slotBrowseSource();
    void slotDecryptAndSave();

private:
    Ui::DecryptWorkspaceDialog *ui;
    QString m_sourceFile;
};

#endif // DECRYPTWORKSPACEDIALOG_H
