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
	- Regular polling
	- SailfishOS notifications
* Chat with any conversation
	- loading most recent history
	- constant long-polling of the currently open conversation
	- automatically sets the read marker
	- display of each messages author, avatar, text and date
	- display of the message replied to, if applicable
	- sets displaynames of mentions, actors
	- recognize and format links
	- stores the last seen message id per conversation
	- view list of participants, click to mention
	- message context menu to copy, mention and reply
	- file preview, download, open (externally)
* Online only, nothing is stored on the device except:
	- account data (host, login, token, user ID plus generated account id and name)
	- last seen message id per conversation

## Please note

* Some account information is stored in plain text (login name, user id, host url). The token/password is stored securely with Sailfish Secrets as of 1.0.0 Alpha 7(!). Previously the password was stored in plain text. With Alpha 7 it is migrated automatically to Sailfish Secrets.
* I am not a seasoned C++/Qt developer, quite the opposite, I am learning it on the way.
* A lot of thanks go to other open source app developers in whose code bases I could find inspiration and solutions!