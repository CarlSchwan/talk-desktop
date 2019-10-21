# Nextcloud Talk for Sailfish OS

Nextcloud Talk is a fully on-premises audio/video and chat communication service. It features web and mobile apps and is designed to offer the highest degree of security while being easy to use.

Nextcloud Talk lowers the barrier for communication and lets your team connect any time, any where, on any device, with each other, customers or partners.

## Sailfish OS App

Nextcloud Talk for Sailfish OS is a hobby project in development and currently in an alpha stage. Due to limitations on he available components on the operating system, video and audio chat cannot be supported as of now. It is merely a text chat application thus.

Due to the earlier development stage, plus refrain from sending bug reports. Code contributions however are very welcome! 

## Features

This is a complete list of features. What is not mentioned, does not work as of now.

* Legacy-type login with host, username and password. Please create an app password on Nextcloud and use it.
* Multi-account support
* List of all conversations, sorted by activity. It contains an unread counter, which is highlighted when an unread mention is present.
	- There is no polling right now. Enter a conversation and leave it for a refresh.
	- A busy indicator shows whether conversations are being (re)loaded
* Chat with any conversation
	- loading of the whole history
	- automatically sets the read marker
	- display of each messages author and text
	- sets displaynames of mentions, actors
	- constant long-polling of the currently open conversation
* Online only, nothing but the accounts are stored on the device
	
## Please note

* Account information is saved in a plain text file, for reasons. One of them was having reusability for other types of apps in mind (contrary to setting up the same account everywhere again), the lack of being able to add an account profile on Sailfish OS generally, and the lack of a credentials store in Sailfish OS (but there is something coming).
* I am not a seasoned C++/Qt developer, quite the opposite, I am learning it on the way.