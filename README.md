# node-freefare

NodeJS binding of LibFreefare. [LibFreefare](https://github.com/nfc-tools/libfreefare) is a library to manipulate Mifare NFC smart cards (Classic, DESFire, Ultralight, etc) over LibNFC. Your reader has to be compatible with LibNFC to work with this Library (Check out the [compatibility matrix](http://nfc-tools.org/index.php?title=Devices_compatibility_matrix) to know if your device is compatible)

Due to lack of time and testing material, the binding is not complete :

* Mifare Ultralight : fully Supported
* Mifare Classic 1K/4K : Partially supported (enough to authenticate, write and read a data block)
* Mifare DESFire : Partially supported (enough to select an application, authenticate in DES/3DES and read/write on a file)

If you need Freefare function which are currently not bound, submit an issue, a pull request or contact me by email.

### Installation

You need at least NodeJS v6. Go to [NodeJs website](https://nodejs.org/en/download/package-manager/) to know how to get it for your system.

You also need LibFreefare. Under debian you can do

```
apt-get install libfreefare0 libfreefare-dev
```

Finally [use npm.](https://docs.npmjs.com/cli/install) to install this package

```
npm install freefare
```

### Example

You can find examples under the `examples/` directory


## API
### Class: Freefare
When a Freefare object is created, it automatically initialize LibNFC. Once initialized, you can list available NFC devices.

#### Freefare.listDevices()

Give a list of available NFC devices

**Returns**: `Promise.&lt;Array.&lt;Device&gt;&gt;`, A promise to the `Device` list.


### Class: Device
A NibNFC compatible device that can read NFC tag.
The `open()` method have to be executed before any other.

#### Device.open()

Open device for further communication

**Returns**: `Promise`, A promise to the end of the action.

#### Device.close()

Close device to release memory and device

**Returns**: `Promise`, A promise to the end of the action.

#### Device.listTags()

List of detected tags

**Returns**: `Promise.&lt;Array.&lt;(Tag|MifareUltralightTag|MifareClassicTag|MifareDesfireTag)&gt;&gt;`, A promise to the list of `Tag`


### Class: Tag
A Freefare compatible NFC tag

#### Tag.getType()

Get Tag type

**Returns**: `string`, The tag type between `MIFARE_CLASSIC_1K`, `MIFARE_CLASSIC_4K`, `MIFARE_DESFIRE`, `MIFARE_ULTRALIGHT`, `MIFARE_ULTRALIGHT_C`

#### Tag.getFriendlyName()

Get Tag friendly name

**Returns**: `string`, The tag friendly name

#### Tag.getUID()

Get Tag UID

**Returns**: `string`, The tag UID


### Class: MifareUltralightTag
A MIFARE Ultralight tag

**Extends**: `Tag`

#### MifareUltralightTag.open()

Open tag for further communication

**Returns**: `Promise`, A promise to the end of the action.

#### MifareUltralightTag.close()

Close tag to release memory and device

**Returns**: `Promise`, A promise to the end of the action.

#### MifareUltralightTag.read(page)

Read a page on the tag

**Parameters**

* **page**: `Number`, The page number between 0 and 11

**Returns**: `Promise.&lt;Buffer&gt;`, A promise to the read data (in a 4 bytes buffer)

#### MifareUltralightTag.write(page, buf)

Write a page on the tag

**Parameters**

* **page**: `Number`, The page number between 0 and 11
* **buf**: `Buffer`, A buffer of 4 bytes to be written on the page

**Returns**: `Promise`, A promise to the end of the action.


### Class: MifareClassicTag
A MIFARE Classic tag

**Extends**: `Tag`


#### MifareClassicTag.open()

Open tag for further communication

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.close()

Close tag to release memory and device

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.authenticate(block, key, keyType)

After openning the tag, an authentication is required for further operation.

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)
* **key**: `Buffer`, The key
* **keyType**: `String`, "A" or "B"

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.read(block)

Read the given block

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)

**Returns**: `Promise.&lt;Buffer&gt;`, A promise to the read data (in a 16 bytes buffer)

#### MifareClassicTag.write(block, data)

Write on the given block

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)
* **data**: `Buffer`, A 16 byte buffer (for 1k)

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.initValue(block, value, adr)

Initialize a value block, with the given value and address

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)
* **value**: `Number`, The Int32 value that will be stored
* **adr**: `String`, A 1 byte address which can be used to save the storage address of a block, when implementing a powerful backup management

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.readValue(block)

Read a value block

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)

**Returns**: `Promise.&lt;Object&gt;`, A promise to an object containing value and adr : `{adr: 0, value: 0}`

#### MifareClassicTag.incrementValue(block, amount)

Increment the block value by a given amount and store it in the internal data register

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)
* **amount**: `Number`, The amount that will be added to the value

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.decrementValue(block, amount)

Decrement the block value by a given amount and store it in the internal data register

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)
* **amount**: `Number`, The amount that will be remove from the value

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.restoreValue(block)

Put a block value from in the internal data register

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)

**Returns**: `Promise`, A promise to the end of the action.

#### MifareClassicTag.transferValue(block)

Restore the internal data register to the given block value

**Parameters**

* **block**: `Number`, The block number between 0 and 63 (for 1k)

**Returns**: `Promise`, A promise to the end of the action.


### Class: MifareDesfireTag
A MIFARE DESFire tag

**Extends**: `Tag`


#### MifareDesfireTag.open()

Open tag for further communication

**Returns**: `Promise`, A promise to the end of the action.

#### MifareDesfireTag.close()

Close tag to release memory and device

**Returns**: `Promise`, A promise to the end of the action.

#### MifareDesfireTag.authenticateDES(KeyNum, key)

Authenticate with a DES key

**Parameters**

* **KeyNum**: `Number`, The number of the key
* **key**: `Buffer`, The 8 byte key

**Returns**: `Promise`, A promise to the end of the action.

#### MifareDesfireTag.authenticate3DES(KeyNum, key)

Authenticate with a 3DES key

**Parameters**

* **KeyNum**: `Number`, The number of the key
* **key**: `Buffer`, The 16 byte key

**Returns**: `Promise`, A promise to the end of the action.

#### MifareDesfireTag.getApplicationIds()

List application IDs (AID)

**Returns**: `Promise.&lt;Array.&lt;Number&gt;&gt;`, A promise to the AID list

#### MifareDesfireTag.selectApplication(aid)

Select an application

**Parameters**

* **aid**: `Number`, Application id

**Returns**: `Promise`, A promise to the end of the action.

### MifareDesfireTag.getFileIds()

List file IDs (AID)

**Returns**: `Promise.&lt;Array.&lt;Number&gt;&gt;`, A promise to the File ID list

### MifareDesfireTag.read(file, offset, length)

Read the given file

**Parameters**

* **file**: `Number`, The file ID
* **offset**: `Number`, The number of bytes before we start reading in the file
* **length**: `Number`, The number of bytes we will read in the file

**Returns**: `Promise.&lt;Buffer&gt;`, A promise to the read data

### MifareDesfireTag.write(file, offset, length, data)

Write on the given file

**Parameters**

* **file**: `Number`, The file ID
* **offset**: `Number`, The number of bytes before we start reading in the file
* **length**: `Number`, The number of bytes we will read in the file
* **data**: `Buffer`, A data buffer

**Returns**: `Promise`, A promise to the end of the action.
