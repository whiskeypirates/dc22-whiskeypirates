/*
 * i2c_interrupt.h
 *
 * Created: 6/13/2014 1:53:36 AM
 *  Author: true
 */ 


#ifndef I2C_INTERRUPT_H_
#define I2C_INTERRUPT_H_


ISR(TWI_vect)
{
	// i2c is low priority - allow it to be interrupted
	// sei();
	
	// figure out wtf to do
	switch (TWSR & 0xf8) {	// TWI state
		// ** SLAVE RECEIVE ** //
		case 0x60:			// slave addr+write receive to valid address
		case 0x68: {		// slave addr+write arbitration lost?
			// nothing to do. we're ready to receive.
			break;
		}
		/*
		case 0x70:
		case 0x78: {
			// unused: this is for genaddr mode.
		}
		*/
		case 0x80: {		// slave data receive w/ACK returned
			if (pirate_process_data(TWDR)) {
				// I wanted to send a NAK, but TWI already sent an ACK. oh fucking well.
				// send NACK if any further packets are received
				i2c_disable_slave();
			}
			break;
		}
		/*case 0x90: {		// slave genaddr data receive w/ACK returned
			// not using this
		}*/
		
		case 0x88: 			// slave data receive w/NACK returned, no longer addressed
		//case 0x98:		// slave genaddr data receive w/NACK returned, no longer addressed
		case 0xa0: {		// STOP asserted; done receiving? no longer addressed
			// we can re-enable the slave...
			i2c_enable_slave();
			break;
		}
		
		// ** SLAVE TRANSMIT ** //
		case 0xa8: {		// slave read request to valid address
			// try to send data
			pirate_data_send();
			break;
		}
		case 0xb8: {		// slave read continues, master ACK'd last packet
			// do we have a mode in progress?
			// NOTE: by default, slave tx modes won't unset, but slave mode will be disabled
			//       when the last packet is sent, and re-enabled on the following ACK/NACK.
			pirate_data_send();
			break;
		}
		
		case 0xb0:			// arbitration lost
			// I don't even know how the fuck this happened
		case 0xc0:			// slave read ends, master doesn't want more data
			// this usually happens after we've sent data and master doesn't want any more.
			// if we had more data, tough shit, master doesn't want it. so we don't send it.
		case 0xc8: {		// slave read ends, master wants more data though
			// this usually happens after we're done sending, but master is requesting more.
			// but slave mode is disabled, so don't send anything.
			// we need to make sure send mode is disabled, and we need to re-enable slave mode.
			i2c_enable_slave();
			break;
		}
		
		// ** ERROR AND UNHANDLED ** //
		case 0x00: {			// bus error
			// set TWSTO, clear TWINT (by setting) as per datasheet
			TWCR |= (_BV(TWSTO)) | (_BV(TWINT));
			// and make sure slave is enabled
			i2c_enable_slave();
			break;
		}
		default: {
			// something unhandled? fuck it, we want to be a slave.
			// reset everything and start over.
			i2c_enable_slave();
		}
	}
	
	// done with this TWI bit
	i2c_clear_int_flag();
}


#endif /* MAIN_INTERRUPT_H_ */