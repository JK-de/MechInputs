/*--------------------------------------------------------------------
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
--------------------------------------------------------------------*/

#ifndef QEIX4_H
#define QEIX4_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class QEIx4
{
public:

	/** constructor of QEIx4 object
	*/
	QEIx4();

	/** destructor of QEIx4 object
	*/
	~QEIx4();

	void begin(int nMode = 4);

	/** Gets the actual counter value.
	*
	* @return        Actual counter value
	*/
	long read() {
		return _counter;
	}

	/** Gets the actual counter value as int operator.
	*
	* @return        Actual counter value as int operator
	*/
	operator long() {   // int-Operator
		return _counter;
	}

	/** Sets the counter value at actual encoder position to given value.
	*
	* @param        Counter value
	*/
	void write(long counter) {
		_counter = counter;
	}

	/** Sets the counter value at actual encoder position to given value as assign operator.
	*
	* @param        Counter value
	*/
	int operator= (long counter) {   // Assign-Operator
		write(counter);
		return counter;
	}

	/** Polls the state machine manually and updates the counter value.
	*/
	void doProcess(bool bInputA, bool bInputB, bool bInputI = false);


	/** Sets the flag for zeroing on next high on index pin while AB lines triggers next counting. The trigger calls tha callback function in which the counter can be set to zero or the actual counter can be latched in for later offset calculation
	*
	* @param        Flag for triggering. Set to 1 for call the attached callback. It is reseted after this call
	*/
	void setIndexTrigger(bool bIndexTrigger) {
		_bIndexTrigger = bIndexTrigger;
	}

	bool hasChanged();

protected:

	long _counter;
	short _state;
	short _eMode;
	bool _bIndexTrigger;
	bool _bHasChanged;


private:
	static short _modeLUT[32];
};



#endif // QEIX4_H
