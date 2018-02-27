// This example is used with the NTAG216, or the NTAG21x devices without authentication.
// On the first detection of a card it will attempt a read and if it suceeds, write some data
// and enable authentication. If the read fails it assumes authentication is enabled and
// attempts and authentication. On success it reads some data and disabled authentication.
'use strict';

const Freefare = require('../index');

// Initialise Library
const freefare = new Freefare();

process.on('unhandledRejection', function(error, p) {
	console.log('Unhandled Rejection at: Promise', p, 'reason:', error);
});

// The 4 byte buffer for the tag password.
const kPassword = Buffer.from([0x11, 0x22, 0x33, 0x44]);
// The 2 pack bytes. These must be the same when setting auth and authenticating.
// Consider them as an extra 2 bytes of password but not as secure.
const kPack = Buffer.from([0xaa, 0xaa]);

async function tagAdded(device, tag) {
	if (tag.getType() == 'NTAG_21x') {
		await tag.open();
		await tag.getInfo();
		const subtype = await tag.getSubType();
		console.log(`${tag.getFriendlyName()} - ${tag.getUID()} - ${subtype}`);
		try {
			await tag.write(0x27, Buffer.from([0x01, 0x02, 0x03, 0x04]));
			console.log('Read data from 0x27', await tag.read(0x27));
			// If the read suceeds then do set auth.
			try {
				await tag.setAuth(kPassword, kPack, 0x00, true);
				console.log('Tag auth set');
			} catch (err) {
				console.log('Unable to set tag auth', err);
			}
		} catch (err) {
			console.log('Unable to read from tag, attempting auth', err);
			try {
				// If other commands are run before this then the tag must be re-opened.
				await tag.close();
				await tag.open();
				await tag.authenticate(kPassword, kPack);
				console.log('Tag authentication success, attempting read');
				console.log('Read data from 0x27', await tag.read(0x27));
				
				console.log('Disabling auth');
				try {
					await tag.disableAuth();
				} catch (err) {
					console.log('Failed to remove authentication');
				}
			} catch (err) {
				console.log('Tag authenticate failed', err);
			}
		}

		await tag.close();
	} else {
		console.log(`${tag.getFriendlyName()} - ${tag.getUID()}`);
	}
	startPoll(device);
}

let lastTag = null;
const kCardTimeout = 5 * 1000;
async function listTags(device) {
	try {
		const tag = await device.poll();
		if (tag) {
			if (!lastTag || Date.now() - lastTag.time > kCardTimeout || lastTag.uid != tag.getUID()) {
				lastTag = {
					time: Date.now(),
					uid: tag.getUID()
				};
				try {
					console.time('tagAdded');
					await tagAdded(device, tag);
					console.timeEnd('tagAdded');
				} catch (err) {
					console.log('tagAdded call failed', err);
					startPoll(device);
				}
			} else {
			   startPoll(device); 
			}
		}
	} catch (err) {
		console.log('Failed to list tags', err);
	}
}

function startPoll(device) {
	setTimeout(function() {
		listTags(device);
	}, 0);
}

(async function() {
	const devices = await freefare.listDevices();
	await devices[0].open();
	startPoll(devices[0]);
})();
