/*
 * symbolprogoutput.cpp, part of the Inspectrum project
 *
 *  Created on: Aug 27, 2025
 *      Author: Pat Deegan
 *  Copyright (C) 2025 Pat Deegan, https://psychogenic.com
 */

#include "symbolprogoutput.h"
SymbolProgOutput::SymbolProgOutput(const QString &output, QWidget *parent, size_t width, size_t height) :
		QDialog(parent)
{
	setWindowTitle("Program Output");
	setMinimumSize(width, height);

	// Create layout
	QVBoxLayout *layout = new QVBoxLayout(this);

	// Create text edit for output (scrollable)
	textEdit = new QTextEdit(this);
	textEdit->setReadOnly(true); // Prevent editing
	textEdit->setText(output);   // Set the output text
	layout->addWidget(textEdit);

	// Create buttons
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	QPushButton *copyButton = new QPushButton("Copy to Clipboard", this);
	QPushButton *closeButton = new QPushButton("Close", this);
	buttonLayout->addWidget(copyButton);
	buttonLayout->addWidget(closeButton);
	layout->addLayout(buttonLayout);

	// Connect buttons
	connect(copyButton, &QPushButton::clicked, this,
			&SymbolProgOutput::copyToClipboard);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

	// Make dialog resizable
	setSizeGripEnabled(true);
}

void SymbolProgOutput::copyToClipboard() {
	textEdit->selectAll();
	textEdit->copy();
	textEdit->textCursor().clearSelection(); // Deselect after copying
}

