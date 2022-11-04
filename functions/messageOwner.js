const aws = require('aws-sdk');
const sqs = new aws.SQS({ region: 'us-west-2' });
const sns = new aws.SNS();
const initWoprStep = require('./helpers/initHoneyBadgerStep');
const dig = require('./helpers/dig');
const getParams = require('./helpers/getParams');

exports.handler = async (event, context) => {
  const { respEvent, log, skipStep } = await initWoprStep(event, context)
  if(skipStep){ return respEvent}

  const managerPhoneStr = dig(event, 'managerInfo', 'phone');
  const assManagerPhoneStr = dig(event, 'managerInfo', 'asstPhone');
  const featureFlag = await getParams(`${process.env.stage}/featureFlag`) || {};
  const executivePhones = featureFlag.executivePhones;

  console.log('CFMCO: ', JSON.stringify(event.event.CMFCustomerOrder.Payload.OrderHeader));
  let { LastModifiedBy, OrderID, VehiclePlateID, VehicleDescription, Review, ReviewDate, CustomerLastName, CustomerFirstName, OrderStatusCode } = event.event.CMFCustomerOrder.Payload.OrderHeader;
  let message = '';

  /* 
  {Internal Commnets}
  1 - License plate was not entered on order.
  2 - License plate vehicle identified does not match vehicle on the order.
  3 - License plate is invalid and could not be identified.
  */

  if(VehiclePlateID === undefined) {VehiclePlateID = ''};
  if(VehicleDescription === undefined) {VehicleDescription = ''};

  if(OrderStatusCode == 'N') {OrderStatusCode = 'New'};
  if(OrderStatusCode == 'C') {OrderStatusCode = 'Completed'};
  const phones = process.env.stage === 'prod' ? managerPhone : executivePhones;
  const personalNumber = '5038698875';
  if (Review == 'Negative') {
    message = `Negative Review Left On: ${ReviewDate}\nWO: ${OrderID}\nCustomer Name: ${CustomerFirstName} ${CustomerLastName}\nLicense ID: ${VehiclePlateID}\nVehicle Description: ${VehicleDescription}\nEmployee: ${LastModifiedBy}`;

    //'+15036799935'
  //   const phone = process.env.stage === 'prod' ? managerPhone : false
  const phoneList = (phones || '').split(',');

  for(const phone of phoneList) {
    const phoneNumber = `+1${personalNumber}`;
    console.log('Text to Manager:', phoneNumber);
    await sendToPhone(phoneNumber, message);
  }

    console.log(message);
    console.log('BADGERING MESSAGE SENT.') 
  } else {
    message = `WO: ${OrderID}\nCustomer Name: ${CustomerFirstName} ${CustomerLastName}\nLicense ID: ${VehiclePlateID}\nVehicle Description: ${VehicleDescription}\nEmployee: ${LastModifiedBy}`;

    //'+15036799935'
  //   const phone = process.env.stage === 'prod' ? managerPhone : false
  const phoneList = (phones || '').split(',');
  for(const phone of phoneList) {
    const phoneNumber = `+1${personalNumber}`;
    console.log('Text to Manager:', phoneNumber);
    // await sendToPhone(phoneNumber, message);
  }
    console.log(message);
    console.log('BADGERING MESSAGE NOT SENT.') 
  }
};

const sendToPhone = (phoneNumber, message) => {
  const params = {
    Message: message,
    PhoneNumber: phoneNumber 
  }
  return sns.publish(params).promise();
}