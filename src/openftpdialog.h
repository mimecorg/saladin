/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2014 Michał Męciński
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef OPENFTPDIALOG_H
#define OPENFTPDIALOG_H

#include <QDialog>

class OpenFtpDialog : public QDialog
{
    Q_OBJECT
public:
    OpenFtpDialog( QWidget* parent );
    ~OpenFtpDialog();

public:
    void setPath( const QString& path );
    void setUser( const QString& user );
    void setPassword( const QString& password );

    QString path();

public: // overrides
    void accept();

private:
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpinBox;
    QLineEdit* m_pathEdit;
    QCheckBox* m_anonymousCheckBox;
    QLineEdit* m_userEdit;
    QLineEdit* m_passwordEdit;
};

#endif
