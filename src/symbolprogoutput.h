/*
 * symbolprogoutput.h, part of the Inspectrum project
 *
 *  Created on: Aug 27, 2025
 *      Author: Pat Deegan
 *  Copyright (C) 2025 Pat Deegan, https://psychogenic.com
 */

#pragma once
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

class SymbolProgOutput : public QDialog
{
    Q_OBJECT
public:
    SymbolProgOutput(const QString& output, QWidget* parent = nullptr, size_t width = 700, size_t height = 500);

private slots:
    void copyToClipboard();

private:
    QTextEdit* textEdit;
};

