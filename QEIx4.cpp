/*-------------------------------------------------------------------------
  Arduino library to ...

  Written by Jochen Krapf,
  contributions by ... and other members of the open
  source community.

  -------------------------------------------------------------------------
  This file is part of the MechInputs library.

  MechInputs is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  MechInputs is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with MechInputs.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "QEIx4.h"


// bit masks for state machine - don't change!!!
#define QEIx4_STATE   0xC
#define QEIx4_MASK    0x1C
#define QEIx4_1x_INC  0x0100
#define QEIx4_2x_INC  0x0200
#define QEIx4_4x_INC  0x0400
#define QEIx4_1x_DEC  0x1000
#define QEIx4_2x_DEC  0x2000
#define QEIx4_4x_DEC  0x4000
#define QEIx4_1x_MASK 0x11FF
#define QEIx4_2x_MASK 0x33FF
#define QEIx4_4x_MASK 0x77FF
#define QEIx4_DIR     0x20
#define QEIx4_A       1
#define QEIx4_B       2
#define QEIx4_AB      3
#define QEIx4_S0      0x0
#define QEIx4_S1      0x4
#define QEIx4_S2      0x8
#define QEIx4_S3      0xC
#define QEIx4_CCW     0
#define QEIx4_CW      0x10
#define QEIx4_IS_CHG  0x7700
#define QEIx4_IS_INC  0x0700
#define QEIx4_IS_DEC  0x7000
#define QEIx4_IS_DIR  0x20

// state machine for decoting - don't change!!!
short QEIx4::_modeLUT[32] = {
	// act state S0 in CCW direction
	QEIx4_CCW | QEIx4_S0,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A  | QEIx4_4x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB | QEIx4_1x_DEC,
	// act state S1 in CCW direction
	QEIx4_CCW | QEIx4_S1,
	QEIx4_CCW | QEIx4_S1 | QEIx4_A,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B  | QEIx4_4x_DEC,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB | QEIx4_1x_INC | QEIx4_DIR,
	// act state S2 in CCW direction
	QEIx4_CCW | QEIx4_S1 |            QEIx4_2x_DEC,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B  | QEIx4_4x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S2 | QEIx4_AB,
	// act state S3 in CCW direction
	QEIx4_CW  | QEIx4_S0 |            QEIx4_2x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A  | QEIx4_4x_DEC,
	QEIx4_CCW | QEIx4_S3 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB,

	// act state S0 in CW direction
	QEIx4_CW  | QEIx4_S0,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A  | QEIx4_4x_INC,
	QEIx4_CW  | QEIx4_S0 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB | QEIx4_1x_DEC | QEIx4_DIR,
	// act state S1 in CW direction
	QEIx4_CW  | QEIx4_S1,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B  | QEIx4_4x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB | QEIx4_1x_INC,
	// act state S2 in CW direction
	QEIx4_CCW | QEIx4_S1 |            QEIx4_2x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S2 | QEIx4_A,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B  | QEIx4_4x_INC,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB,
	// act state S3 in CW direction
	QEIx4_CW  | QEIx4_S0 |            QEIx4_2x_INC,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A  | QEIx4_4x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B,
	QEIx4_CW  | QEIx4_S3 | QEIx4_AB
};


//#define DEB(x) printf (x)
#define DEB(x)

///////////////////////////////////////////////////////////////////////////////

QEIx4::QEIx4()
{
	_state = 0;
	begin();
}

///////////////////////////////////////////////////////////////////////////////

QEIx4::~QEIx4()
{
}

///////////////////////////////////////////////////////////////////////////////

void QEIx4::begin(int nMode = 4)
{
	_counter = 0;

	if (nMode==1)
		_eventMask = QEIx4_1x_MASK;
	else if (nMode == 2)
		_eventMask = QEIx4_2x_MASK;
	else
		_eventMask = QEIx4_4x_MASK;
}

///////////////////////////////////////////////////////////////////////////////

void QEIx4::doProcess(bool bInputA, bool bInputB, bool bInputI);
{
	int pinA, pinB;

	DEB(".");
	_state &= QEIx4_MASK;
	if (bInputA) _state |= QEIx4_A;
	if (bInputB) _state |= QEIx4_B;

	_state = _modeLUT[_state];   // magic is done by lookup-table

	if (_state & QEIx4_CHG) {   // is any change?
		bool bCounterChange = false;

		if (_state & QEIx4_INC) {   // is moved foreward?
			_counter++;
			bCounterChange = true;
		}
		if (_state & QEIx4_DEC) {   // is moved backward?
			_counter--;
			bCounterChange = true;
		}


		if (_bIndexTrigger && bCounterChange && bInputI) {   // is index pin triggered?
			_bIndexTrigger = false;
			//fPointerIndexTrigger.call(_counter);
		}

		if (bCounterChange) {   // has counter changed?
			fPointerCounterChange.call(_counter);
			//if (_state & QEIx4_DIR)
			//	fPointerDirectionChange.call(_counter);
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
