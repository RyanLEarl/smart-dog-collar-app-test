class Logger {
  constructor() {
    this.isProduction = process.env.stage === 'prod';
    this.logLevelMap = { debug: 3, info: 2, error: 1 };
    this.logLevel = this.setLogLevel();
  }

  setLogLevel() {
    return this.logLevelMap[process.env.logLevel];
  }

  debug(...message) {
    if (this.isProduction && this.logLevel < 3) {
      return;
    }
    console.log(
      `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
    );
  }

  info(...message) {
    if (this.isProduction && this.logLevel < 2) {
      return;
    }
    console.log(
      `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
    );
  }

  error(...message) {
    if (this.isProduction && this.logLevel < 1) {
      return;
    }
    console.log(
      `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
    );
  }

  audit(...message) {
    if (this.isProduction) {
      console.log(
        `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
      );
    }
    console.log(
      `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
    );
  }

  log(...message) {
    if (this.isProduction) {
      return;
    }
    console.log(
      `${process.env.stackName.toUpperCase()}: ${JSON.stringify(message)}`
    );
  }
}

module.exports = { Logger };