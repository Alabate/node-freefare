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
					console.log('Authenticate on block 6');
					return tag.authenticate(6, new Buffer([0xff, 0xff, 0xff, 0xff, 0xff, 0xff]), "B");
				})
				.then(() => {
					// -----------
					console.log('Read block 6');
					return tag.read(6);
				})
				.then((data) => {
					console.log('  ', data)
					// -----------
					console.log('Write block 6');
					return tag.write(6, new Buffer([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10]));
				})
				.then(() => {
					// -----------
					console.log('Read block 6');
					return tag.read(6);
				})
				.then((data) => {
					console.log('  ', data)
					// -----------
					console.log('Init value on block 6');
					return tag.initValue(6, 1234567890, 242);
				})
				.then(() => {
					// -----------
					console.log('Read value on block 6');
					return tag.readValue(6);
				})
				.then((res) => {
					console.log('  ', res)
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
