'use strict';

const Freefare = require('../index');

// initialize Library
let freefare = new Freefare()

// Ask compatible and available devices
freefare.listDevices()
.then(devices => {

	// Open all found devices
	for(let device of devices) {
		console.log('Device found : ' + device.name);
		device.open()
		.then(() => {
			console.log('Device opened : ' + device.name + '')

			// List available tag on each devices
			return device.listTags();
		})
		.then((tags) => {
			console.log('Tag list (' + device.name + ') :');
			for(let tag of tags) {
				console.log('----------------------');
				console.log(tag.getFriendlyName());
				console.log(tag.getUID());
			}
			console.log('----------------------');

			// Open mifare ultralight tags
			// TODO Open only mifare ultralight
			for(let tag of tags) {
				tag.open()
				.then(() => {
					console.log('Tag opened (' + tag.getUID() + ' on ' + device.name + ')');

					// Read last page
					return tag.read(15);
				})
				.then((data) => {
					console.log('Page 15 read (' + tag.getUID() + ' on ' + device.name + ') :');
					console.log(data)

					// Write last page
					return tag.write(15, new Buffer([0x01, 0x02, 0x03, 0x04]));
				})
				.then(() => {
					console.log('Page 15 written (' + tag.getUID() + ' on ' + device.name + ')');

					// Read last page again
					return tag.read(15);
				})
				.then((data) => {
					console.log('Page 15 read (' + tag.getUID() + ' on ' + device.name + ') :');
					console.log(data)

					// Close tag
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
