const aws = require('aws-sdk');
const sqs = new aws.SQS({ region: 'us-west-2' });
const sns = new aws.SNS();
const initWoprStep = require('./helpers/initCollarStep');
const dig = require('./helpers/dig');
const getParams = require('./helpers/getParams');

exports.handler = async (event, context) => {
  const { respEvent, log, skipStep } = await initWoprStep(event, context)
  if(skipStep){ return respEvent}

  // const featureFlag = await getParams(`${process.env.stage}/featureFlag`) || {};

  let message = '';
  console.log('Data: ', JSON.stringify(event.Payload));
  let { Time, Severity, dogName } = event.Payload;
  phones = '5038698875';
  const personalNumber = '5038698875';
  if (Severity >= 0.5) {
    message = `Testing${personalNumber}`;

  const phoneList = (phones || '').split(',');

  for(const phone of phoneList) {
    const phoneNumber = `+1${personalNumber}`;
    console.log('Text to Owner:', phoneNumber);
    await sendToPhone(phoneNumber, message);
  }

    console.log(message);
    console.log('MESSAGE TO OWNER SENT.') 
  } else {
    message = `Testing`;

  const phoneList = (phones || '').split(',');
  for(const phone of phoneList) {
    const phoneNumber = `+1${personalNumber}`;
    console.log('Text to Owner:', phoneNumber);
    // await sendToPhone(phoneNumber, message);
  }
    console.log(message);
    console.log(' MESSAGE TO OWNER NOT SENT.') 
  }
};

const sendToPhone = (phoneNumber, message) => {
  const params = {
    Message: message,
    PhoneNumber: phoneNumber 
  }
  return sns.publish(params).promise();
}