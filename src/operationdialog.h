/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2012 Michał Męciński
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

#ifndef OPERATIONDIALOG_H
#define OPERATIONDIALOG_H

#include <QDialog>

class MultiRenameWidget;

class OperationDialog : public QDialog
{
    Q_OBJECT
public:
    enum Flag
    {
		WithName = 0x1,
		WithPattern = 0x2,
		WithLocation = 0x4,
		WithSource = 0x8,
		WithTarget = 0x10,
		WithCheckBox = 0x20,
		WithMultiRename = 0x40,
		CanEditName = 0x80,
		CanRename = 0x100
    };

    Q_DECLARE_FLAGS( Flags, Flag )

public:
    OperationDialog( Flags flags, QWidget* parent );
    ~OperationDialog();

public:
    void setPromptPixmap( const QPixmap& pixmap );
    void setPrompt( const QString& text );

    void setName( const QString& text );
    void setPattern( const QString& text );
    void setLocation( const QString& text );
    void setSource( const QString& text );
    void setTarget( const QString& text );

    QString name() const;
    QString pattern() const;

    void setCheckBoxText( const QString& text );

    void setCheckBoxChecked( bool checked );
    bool checkBoxChecked() const;

    void setNames( const QStringList& names );
    const QStringList& names() const { return m_names; }

public: // overrides
    void accept();

private slots:
    void rename();

private:
    void expandTo( const QSize& newSize );

private:
    QLabel* m_promptLabel;
    QLabel* m_promptPixmap;

    QBoxLayout* m_mainLayout;

    QLineEdit* m_nameEdit;
    QLineEdit* m_patternEdit;
    QLineEdit* m_locationEdit;
    QLineEdit* m_sourceEdit;
    QLineEdit* m_targetEdit;

    QCheckBox* m_checkBox;

    MultiRenameWidget* m_multiRename;

    QPushButton* m_renameButton;

    QStringList m_names;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( OperationDialog::Flags )

#endif
