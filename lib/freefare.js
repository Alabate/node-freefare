var EventEmitter = require('events').EventEmitter;
var util = require('util');
const assert = require('assert');

var objectwrapper = require('bindings')('freefare');
var freefare = new objectwrapper.Freefare();
var device = new objectwrapper.Device();

const ERROR_INIT_LIBNFC = 10; // TODO move that to binding class from C++
const ERROR_OPEN_DEVICE = 11; // TODO move that to binding class from C++

// This symbol is used to make cpp wrapped object private
var cppObj = Symbol();

/**
* When a Freefare object is created, it automatically initialize LibNFC. Once initialized, you can list available NFC devices.
*
* @class Freefare
*/
class Freefare {

	constructor() {
		let error = freefare.init();
		if(error == ERROR_INIT_LIBNFC) {
			throw Error('Could not initiate LibNFC');
		}
		else if(error){
			throw new Error('Unknown error during LibNFC initialization');
		}
	}

	/**
	* Give a list of available NFC devices
	* @return {Promise<Device[]>} A promise to the `Device` list.
	*/
	listDevices() {
		return new Promise((resolve, reject) => {
			freefare.listDevices((error, deviceList) => {
				if(error){
					reject(new Error('Unknown error during device list'));
				}

				let res = [];
				for (let cppDevice of deviceList) {
					res.push(new Device(cppDevice));
				}
				resolve(res);
			});
		});
	}

}

/**
* A NibNFC compatible device that can read NFC tag.
* The `open()` method have to be executed before any other.
*
* @class Device
*/
class Device {
	constructor(cppDevice) {
		this[cppObj] = cppDevice;
		this.name = this[cppObj].getConnstring();
	}

	/**
	*  Open device for further communication
	* @return {Promise} A promise to the end of the action.
	*/
	open() {
		return new Promise((resolve, reject) => {
			this[cppObj].open((error) => {
				if(error) {
					switch (error) {
						case ERROR_OPEN_DEVICE:
						reject(new Error('LibNFC failed to open the device'));
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Close device to release memory and device
	* @return {Promise} A promise to the end of the action.
	*/
	close() {
		return new Promise((resolve, reject) => {
			this[cppObj].open((error) => {
				if(error) {
					switch (error) {
						case ERROR_OPEN_DEVICE:
						reject(new Error('LibNFC failed to open the device'));
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	parseTag(cppTag) {
		if (!cppTag) {
			return null;
		}
		switch (cppTag.getTagType()) {
			case 'MIFARE_CLASSIC_1K':
			case 'MIFARE_CLASSIC_4K':
				return new MifareClassicTag(cppTag);
			case 'MIFARE_DESFIRE':
				return new MifareDesfireTag(cppTag);
			case 'MIFARE_ULTRALIGHT':
				return new MifareUltralightTag(cppTag);
			case 'NTAG_21x':
				return new NtagTag(cppTag);
			case 'MIFARE_ULTRALIGHT_C':
			default:
				return new Tag(cppTag);
		}
	}

	parseList(list) {
		let res = [];
		for (let cppTag of list) {
			const tag = this.parseTag(cppTag);
			if (tag) {
				res.push(tag);
			}
		}
		return res;
	}

	/**
	* List of detected tags
	* @return {Promise<Array<Tag|MifareUltralightTag|MifareClassicTag|MifareDesfireTag>>} A promise to the list of `Tag`
	*/
	listTags() {
		const $this = this;
		return new Promise((resolve, reject) => {
			this[cppObj].listTags((error, list) => {
				if (error) {
					return reject(new Error(error));
				}

				resolve(this.parseList(list));
			});
		});
	}

	/**
	* Wait infinitely for a tag to be added.
	* @return {Promise<Tag|MifareUltralightTag|MifareClassicTag|MifareDesfireTag>} A promise to of `Tag`
	*/
	poll() {
		const $this = this;
		return new Promise((resolve, reject) => {
			this[cppObj].poll((error, tag) => {
				if (error) {
					return reject(new Error(error));
				}
				resolve(this.parseTag(tag));
			});
		});
	}

	/**
	* Try to abort the current blocking command
	* @return {Promise} A promise to the end of the action.
	*/
	abort() {
		return new Promise((resolve, reject) => {
			this[cppObj].abort((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}
}


/**
* A Freefare compatible NFC tag
*
* @class Tag
*/
class Tag {
	constructor(cppTag) {
		this[cppObj] = cppTag;
	}

	/**
	* Get Tag type
	* @return {string} The tag type between `MIFARE_CLASSIC_1K`, `MIFARE_CLASSIC_4K`, `MIFARE_DESFIRE`, `MIFARE_ULTRALIGHT`, `MIFARE_ULTRALIGHT_C`
	*/
	getType() {
		return this[cppObj].getTagType();
	}

	/**
	* Get Tag friendly name
	* @return {string} The tag friendly name
	*/
	getFriendlyName() {
		return this[cppObj].getTagFriendlyName();
	}

	/**
	* Get Tag UID
	* @return {string} The tag UID
	*/
	getUID() {
		return this[cppObj].getTagUID();
	}
}

/**
* A MIFARE Ultralight tag
*
* @class MifareUltralightTag
* @extends Tag
*/
class MifareUltralightTag extends Tag {
	constructor(cppTag) {
		super(cppTag);
	}

	/**
	* Open tag for further communication
	* @return {Promise} A promise to the end of the action.
	*/
	open() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareUltralight_connect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Close tag to release memory and device
	* @return {Promise} A promise to the end of the action.
	*/
	close() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareUltralight_disconnect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Read a page on the tag
	* @param {Number} page The page number between 0 and 11
	* @return {Promise<Buffer>} A promise to the read data (in a 4 bytes buffer)
	*/
	read(page) {
		assert(page < 16, 'Mifare Ultralight tag have only 16 pages (0 to 15)');
		return new Promise((resolve, reject) => {
			this[cppObj].mifareUltralight_read(page, (error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* Write a page on the tag
	* @param {Number} page The page number between 0 and 11
	* @param {Buffer} buf A buffer of 4 bytes to be written on the page
	* @return {Promise} A promise to the end of the action.
	*/
	write(page, buf) {
		assert(page < 16, 'Mifare Ultralight tag have only 16 pages (0 to 15)');
		assert(buf.length == 4, 'Length of page on Mifare Ultralight tag is 4 bytes');
		return new Promise((resolve, reject) => {
			this[cppObj].mifareUltralight_write(page, buf, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}
}


/**
* A MIFARE Classic tag
*
* @class MifareClassicTag
* @extends Tag
*/
class MifareClassicTag extends Tag {
	constructor(cppTag) {
		super(cppTag);
	}

	/**
	* Open tag for further communication
	* @return {Promise} A promise to the end of the action.
	*/
	open() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_connect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Close tag to release memory and device
	* @return {Promise} A promise to the end of the action.
	*/
	close() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_disconnect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* After openning the tag, an authentication is required for further operation.
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @param {Buffer} key The key
	* @param {String} keyType "A" or "B"
	* @return {Promise} A promise to the end of the action.
	*/
	authenticate(block, key, keyType) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_authenticate(block, key, keyType, (error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* Read the given block
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @return {Promise<Buffer>} A promise to the read data (in a 16 bytes buffer)
	*/
	read(block, key, keyType) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_read(block, (error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* Write on the given block
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @param {Buffer} data A 16 byte buffer (for 1k)
	* @return {Promise} A promise to the end of the action.
	*/
	write(block, data) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_write(block, data, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Initialize a value block, with the given value and address
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @param {Number} value The Int32 value that will be stored
	* @param {String} adr A 1 byte address which can be used to save the storage address of a block, when implementing a powerful backup management
	* @return {Promise} A promise to the end of the action.
	*/
	initValue(block, value, adr) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_initValue(block, value, adr, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Read a value block
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @return {Promise<Object>} A promise to an object containing value and adr : `{adr: 0, value: 0}`
	*/
	readValue(block) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_readValue(block, (error, value, adr) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve({value, adr});
			});
		});
	}

	/**
	* Increment the block value by a given amount and store it in the internal data register
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @param {Number} amount The amount that will be added to the value
	* @return {Promise} A promise to the end of the action.
	*/
	incrementValue(block, amount) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_increment(block, amount, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Decrement the block value by a given amount and store it in the internal data register
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @param {Number} amount The amount that will be remove from the value
	* @return {Promise} A promise to the end of the action.
	*/
	decrementValue(block, amount) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_decrement(block, amount, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Put a block value from in the internal data register
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @return {Promise} A promise to the end of the action.
	*/
	restoreValue(block) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_restore(block, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Restore the internal data register to the given block value
	* @param {Number} block The block number between 0 and 63 (for 1k)
	* @return {Promise} A promise to the end of the action.
	*/
	transferValue(block) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareClassic_transfer(block, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}
}

/**
* A MIFARE DESFire tag
*
* @class MifareDesfireTag
* @extends Tag
*/
class MifareDesfireTag extends Tag {
	constructor(cppTag) {
		super(cppTag);
	}

	/**
	* Open tag for further communication
	* @return {Promise} A promise to the end of the action.
	*/
	open() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_connect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Close tag to release memory and device
	* @return {Promise} A promise to the end of the action.
	*/
	close() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_disconnect((error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Authenticate with a DES key
	* @param {Number} KeyNum The number of the key
	* @param {Buffer} key The 8 byte key
	* @return {Promise} A promise to the end of the action.
	*/
	authenticateDES(KeyNum, key) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_authenticate_des(KeyNum, key, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* Authenticate with a 3DES key
	* @param {Number} KeyNum The number of the key
	* @param {Buffer} key The 16 byte key
	* @return {Promise} A promise to the end of the action.
	*/
	authenticate3DES(KeyNum, key) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_authenticate_3des(KeyNum, key, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}

	/**
	* List application IDs (AID)
	* @return {Promise<Number[]>} A promise to the AID list
	*/
	getApplicationIds() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_getApplicationIds((error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* Select an application
	* @param {Buffer} aid Application id in a 3 bytes Buffer
	* @return {Promise} A promise to the end of the action.
	*/
	selectApplication(aid) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_selectApplication(aid, (error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* List file IDs (AID)
	* @return {Promise<Number[]>} A promise to the File ID list
	*/
	getFileIds() {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_getFileIds((error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}


	/**
	* Read the given file
	* @param {Number} file The file ID
	* @param {Number} offset The number of bytes before we start reading in the file
	* @param {Number} length The number of bytes we will read in the file
	* @return {Promise<Buffer>} A promise to the read data
	*/
	read(file, offset, length) {
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_read(file, offset, length, (error, result) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve(result);
			});
		});
	}

	/**
	* Write on the given file
	* @param {Number} file The file ID
	* @param {Number} offset The number of bytes before we start reading in the file
	* @param {Number} length The number of bytes we will read in the file
	* @param {Buffer} data A data buffer
	* @return {Promise} A promise to the end of the action.
	*/
	write(file, offset, length, data) {
		console.log('hello')
		return new Promise((resolve, reject) => {
			this[cppObj].mifareDesfire_write(file, offset, length, data, (error) => {
				if(error) {
					switch (error) {
						default:
						reject(new Error('Unknown error (' + error + ')'));
					}
				}
				resolve();
			});
		});
	}
}

/**
* A NTAG_21x tag
*
* @class NTAG_21x Tag
* @extends Tag
*/
class NtagTag extends Tag {
	constructor(cppTag) {
		super(cppTag);
	}
    
    wrap(fn) {
		const $arguments = Array.from(arguments).slice(1);
		const $this = this;
        return new Promise(function(resolve, reject) {
			$arguments.push(function(error, data) {
                if (error) {
                    reject(error);
                } else {
                    resolve(data);
                }
            });
			fn.apply($this[cppObj], $arguments);
        });
    }

	/**
	* Open tag for further communication
	* @return {Promise} A promise to the end of the action.
	*/
	open() {
        return this.wrap(this[cppObj].ntag_connect);
	}

	/**
	* Close a tag
	* @return {Promise} A promise to the end of the action.
	*/
    close() {
        return this.wrap(this[cppObj].ntag_disconnect);
    }

	/**
	* Retrieve info such as the sub-type.
	* @return {Promise} A promise to the end of the action.
	*/
    getInfo() {
        return this.wrap(this[cppObj].ntag_getInfo);
    }

	/**
	* Gets the tag sub-type as a string. getInfo must be called first.
	* @return {Promise} A promise to the end of the action which returns the sub-type.
	*/
    getSubType() {
        return this.wrap(this[cppObj].ntag_getType);
    }

	/**
	* Writes a 4 byte buffer to the given page.
	* @param page The given page to write to. Must be between 0x00 and 0xff inclusive.
	* @param buffer A 4 byte buffer to write to the given page.
	* @return {Promise} A promise to the end of the action.
	*/
	write(page, buffer) {
		if (page < 0 || page > 0xff) {
			throw new Error('Invalid page');
		}
		if (!(buffer instanceof Buffer)) {
			throw new Error('Data is not a buffer');
		}
		if (buffer.length != 4) {
			throw new Error('Data is not 4 bytes');
		}
		return this.wrap(this[cppObj].ntag_write, page, buffer);
	}

	/**
	* Reads a 4 byte buffer from the given page.
	* @param page The given page to write to. Must be between 0x00 and 0xff inclusive.
	* @return {Promise} A promise to the end of the action which returns a 4 byte buffer of the page.
	*/
	read(page) {
		if (page < 0 || page > 0xff) {
			throw new Error('Invalid page');
		}
		return this.wrap(this[cppObj].ntag_read, page);
	}

	/**
	* Set a password for writing or reading and writing to tag.
	* @param password A 4 byte buffer representing the password.
	* @param pack A 2 byte buffer representing the password acknowledgement bytes.
	*             These could be considered as an extra 2 password bytes.
	* @param startPage The page to start the protection from 0x00-0xff.
	* @param prot Set to true to enable password protection for reads. False to only password protect writes.
	* @return {Promise} A promise to the end of the action.
	*/
	setAuth(password, pack, startPage, prot) {
		if (!(password instanceof Buffer)) {
			throw new Error('Password is not a buffer');
		}
		if (password.length != 4) {
			throw new Error('Password is not 4 bytes');
		}
		if (!(pack instanceof Buffer)) {
			throw new Error('Pack is not a buffer');
		}
		if (pack.length != 2) {
			throw new Error('Pack is not 2 bytes');
		}
		prot = prot ? true : false;
		if (startPage < 0 || startPage > 0xff) {
			throw new Error('Invalid start page');
		}
		return this.wrap(this[cppObj].ntag_set_auth, password, pack, startPage, prot);
	}

	async removeAuth() {
		await this.setAuth(Buffer.from([0x0, 0x0, 0x0, 0x0]), Buffer.from([0x00, 0x00]), 0xff, false);
	}

	/**
	* Authenticates a tag with a given password.
	* @param password A 4 byte buffer representing the password.
	* @param pack A 2 byte buffer representing the password acknowledgement bytes.
	*             These could be considered as an extra 2 password bytes.
	* @return {Promise} A promise to the end of the action.
	*/
	authenticate(password, pack) {
		if (!(password instanceof Buffer)) {
			throw new Error('Password is not a buffer');
		}
		if (password.length != 4) {
			throw new Error('Password is not 4 bytes');
		}
		if (!(pack instanceof Buffer)) {
			throw new Error('Pack is not a buffer');
		}
		if (pack.length != 2) {
			throw new Error('Pack is not 2 bytes');
		}
		return this.wrap(this[cppObj].ntag_authenticate, password, pack);
	}
}

module.exports = Freefare;
