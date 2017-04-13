'use strict';

const Freefare = require('../index');
var last = 0;
let devices = [];

let freefare = new Freefare();
freefare.listDevices()
.then(devices => {
	for(let device of devices) {
		console.log('Open device ' + device.name);

		device.open()
		.then(() => {
			console.log('Request list of tag (' + device.name + ')');
			return device.listTags();
		})
		.then((tags) => {
			console.log('Tag list (' + device.name + ')');
			for(let tag of tags) {
				console.log('----------------------');
				console.log(tag.getFriendlyName());
				console.log(tag.getUID());
			}
			console.log('----------------------');


			for(let tag of tags) {
				console.log('open tag ' + tag.getUID() + ' on ' + device.name);
				tag.open()
				.then(() => {
					// -----------
					console.log('----------------------');
					console.log('List AID');
					return tag.getApplicationIds();
				})
				.then((data) => {
					for (let aid of data) {
						console.log(aid.toString(16))
					}
					console.log('----------------------');
					// -----------
					console.log('Select application');
					return tag.selectApplication(new Buffer([0xff, 0xff, 0xff]));
				})
				.then(() => {
					// -----------
					console.log('List File IDs');
					return tag.getFileIds();
				})
				.then((data) => {
					console.log('  ', data)
					// -----------
					console.log('Authenticate with a read only-key');
					return tag.authenticate3DES(0x04, new Buffer([0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff]));
				})
				.then(() => {
					// -----------
					console.log('Read file');
					return tag.read(2, 0, 16);
				})
				.then((data) => {
					console.log('  ', data)
					// -----------
					console.log('Authenticate with a write-only key');
					return tag.authenticate3DES(0x03, new Buffer([0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff]));
				})
				.then(() => {
					// -----------
					console.log('Write file');
					return tag.write(2, 0, 16, new Buffer([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0c, 0x0d, 0x0e, 0x0f, 0x10]));
				})
				.then(() => {
					// -----------
					console.log('Authenticate with a read only-key');
					return tag.authenticate3DES(0x04, new Buffer([0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff]));
				})
				.then(() => {
					// -----------
					console.log('Read file');
					return tag.read(2, 0, 16);
				})
				.then((data) => {
					console.log('  ', data)
					// -----------
					console.log('Close tag');
					return tag.close();
				})
				.catch(error => {
					console.log(error);
				});
			}
		})
		.catch(error => {
			console.log(error);
		});
	}
})
.catch(error => {
	console.log(error);
});
